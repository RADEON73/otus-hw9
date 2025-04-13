#pragma once
#include <iostream>
#include <string>
#include <memory>
#include "async.h"
#include "ProcessorManager.h"
#include <iosfwd>
#include <exception>
#include <fstream>
#include <sstream>
/// Справка
static void print_help() {
	std::cout << "Available commands:\n"
		<< "  connect <BS>          - Create new processor with bulk size BS\n"
		<< "  receive <PID> <DATA>  - Send DATA to processor PID\n"
		<< "  disconnect <PID>      - Disconnect processor PID\n"
		<< "  list                  - List active processors\n"
		<< "  help                  - Show this help\n"
		<< "  exit                  - Exit program\n";
}

// Базовый интерфейс команды
class ICommand
{
public:
	virtual ~ICommand() = default;
	virtual void execute() = 0;
};

/// Базовый класс для команд, работающих с процессорами
class ProcessorCommand : public ICommand
{
public:
	ProcessorManager& manager;
	std::istringstream& iss;
	bool interactive;

	ProcessorCommand(ProcessorManager& mgr, std::istringstream& input, bool interactive)
		: manager(mgr), iss(input), interactive(interactive) {
	}

	void logError(const std::string_view message) const
	{
		if (interactive)
			std::cerr << message << std::endl;
	}
};

/// Конкретные команды
class ConnectCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	void execute() override {
		size_t packSize;
		if (iss >> packSize) {
			auto handle = async::connect(packSize);
			manager.addProcessor(handle);
		}
		else
			logError("Error: missing or invalid bulk size");
	}
};

class ReceiveCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	void execute() override {
		int id;
		if (std::string data; iss >> id && getline(iss, data)) {
			size_t pos = 0;
			async::HANDLE target = manager.getProcessor(id);
			if (target && !data.empty()) {
				while ((pos = data.find("\\n", pos)) != std::string::npos) {
					data.replace(pos, 2, "\n");
					pos += 1;
				}
				data.erase(0, data.find_first_not_of(" \t"));
				async::receive(target, data.data(), data.size());
			}
			else
				logError("Error: invalid processor ID or empty data");
		}
		else
			logError("Error: invalid arguments");
	}
};

class DisconnectCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	void execute() override {
		int id;
		if (iss >> id) {
			async::HANDLE target = manager.getProcessor(id);
			if (target) {
				async::disconnect(target);
				manager.removeProcessor(id);
			}
			else
				logError("Error: processor not found");
		}
		else
			logError("Error: missing processor ID");
	}
};

class ListCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	void execute() override {
		if (!interactive)
			return;

		if (manager.empty()) {
			std::cout << "No active processors\n";
		}
		else {
			std::cout << "Active processors:\n";
			for (const auto& [h, id] : manager.getAllProcessors()) {
				std::cout << "  #" << id << " (handle: " << h << ")\n";
			}
		}
	}
};

class HelpCommand : public ICommand
{
	bool interactive;

public:
	explicit HelpCommand(bool interactive) : interactive(interactive) {}

	void execute() override {
		if (interactive)
			print_help();
	}
};

class ExitCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	void execute() override { manager.closeRequest(); }
};

// Фабрика команд
class CommandFactory
{
private:
	ProcessorManager& manager;
	bool interactive;

public:
	CommandFactory(ProcessorManager& mgr, bool interactive)
		: manager(mgr), interactive(interactive) {
	}

	std::unique_ptr<ICommand> createCommand(const std::string& cmd, std::istringstream& iss) {
		if (cmd.empty() || cmd[0] == '#') {
			return nullptr; // Skip empty lines and comments
		}

		try {
			if (cmd == "connect") {
				return std::make_unique<ConnectCommand>(manager, iss, interactive);
			}
			else if (cmd == "receive") {
				return std::make_unique<ReceiveCommand>(manager, iss, interactive);
			}
			else if (cmd == "disconnect") {
				return std::make_unique<DisconnectCommand>(manager, iss, interactive);
			}
			else if (cmd == "list") {
				return std::make_unique<ListCommand>(manager, iss, interactive);
			}
			else if (cmd == "help") {
				return std::make_unique<HelpCommand>(interactive);
			}
			else if (cmd == "exit") {
				return std::make_unique<ExitCommand>(manager, iss, interactive);
			}
			else
				if (interactive) {
					std::cerr << "Error: unknown command. Type 'help' for available commands.\n";
				}
		}
		catch (const std::exception& e) {
			if (interactive) {
			std::cerr << "Command error: " << e.what() << std::endl;
			}
			return nullptr;
		}
		return nullptr;
	}
};

// Основная функция обработки команд
void process_commands_from_stream(std::istream& input, bool interactive = true) {
	ProcessorManager manager;
	CommandFactory factory(manager, interactive);

	if (interactive) {
		std::cout << "Interactive bulk command processor\n";
		print_help();
	}

	std::string line;
	while (getline(input, line)) {
		std::istringstream iss(line);
		std::string cmd;
		iss >> cmd;
		if (auto command = factory.createCommand(cmd, iss))
			command->execute();
		if (manager.isCloseRequested())
			break;
	}
	for (const auto& [h, id] : manager.getAllProcessors())
		async::disconnect(h);
}