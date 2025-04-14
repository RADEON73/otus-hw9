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

/// @brief Выводит справочную информацию о доступных командах
static void print_help() {
	std::cout << "Available commands:\n"
		<< "  connect <BS>          - Create new processor with bulk size BS\n"
		<< "  receive <PID> <DATA>  - Send DATA to processor PID\n"
		<< "  disconnect <PID>      - Disconnect processor PID\n"
		<< "  list                  - List active processors\n"
		<< "  help                  - Show this help\n"
		<< "  exit                  - Exit program\n";
}

/// @brief Базовый интерфейс команды
class ICommand
{
public:
	virtual ~ICommand() = default;

	/// @brief Выполняет команду
	virtual void execute() = 0;
};

/// @brief Базовый класс для команд, работающих с процессорами
/// @details Предоставляет общий функционал для команд, взаимодействующих с ProcessorManager
class ProcessorCommand : public ICommand
{
public:
	ProcessorManager& manager;       ///< Менеджер процессоров
	std::istringstream& iss;        ///< Поток ввода для чтения аргументов команды
	bool interactive;               ///< Флаг интерактивного режима

	/// @brief Конструктор базовой команды процессора
	/// @param mgr Ссылка на менеджер процессоров
	/// @param input Поток ввода для чтения аргументов
	/// @param interactive Флаг интерактивного режима
	ProcessorCommand(ProcessorManager& mgr, std::istringstream& input, bool interactive)
		: manager(mgr), iss(input), interactive(interactive) {
	}

	/// @brief Логирует ошибку в интерактивном режиме
	/// @param message Текст сообщения об ошибке
	void logError(const std::string_view message) const
	{
		if (interactive)
			std::cerr << message << std::endl;
	}
};

/// @brief Команда создания нового процессора
class ConnectCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	/// @brief Создает новый процессор с указанным размером блока
	/// @details Формат команды: connect <BS>
	/// где BS - размер блока команд для обработки
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

/// @brief Команда отправки данных процессору
class ReceiveCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	/// @brief Отправляет данные указанному процессору
	/// @details Формат команды: receive <PID> <DATA>
	/// где PID - идентификатор процессора, DATA - данные для обработки
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

/// @brief Команда отключения процессора
class DisconnectCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	/// @brief Отключает указанный процессор
	/// @details Формат команды: disconnect <PID>
	/// где PID - идентификатор процессора для отключения
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

/// @brief Команда вывода списка активных процессоров
class ListCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	/// @brief Выводит список активных процессоров
	/// @details Работает только в интерактивном режиме
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

/// @brief Команда вывода справки
class HelpCommand : public ICommand
{
	bool interactive; ///< Флаг интерактивного режима

public:
	/// @brief Конструктор команды помощи
	/// @param interactive Флаг интерактивного режима
	explicit HelpCommand(bool interactive) : interactive(interactive) {}

	/// @brief Выводит справочную информацию
	void execute() override {
		if (interactive)
			print_help();
	}
};

/// @brief Команда выхода из программы
class ExitCommand : public ProcessorCommand
{
public:
	using ProcessorCommand::ProcessorCommand;

	/// @brief Инициирует завершение работы программы
	void execute() override { manager.closeRequest(); }
};

/// @brief Фабрика создания команд
class CommandFactory
{
private:
	ProcessorManager& manager; ///< Менеджер процессоров
	bool interactive;          ///< Флаг интерактивного режима

public:
	/// @brief Конструктор фабрики команд
	/// @param mgr Ссылка на менеджер процессоров
	/// @param interactive Флаг интерактивного режима
	CommandFactory(ProcessorManager& mgr, bool interactive)
		: manager(mgr), interactive(interactive) {
	}

	/// @brief Создает команду на основе входных данных
	/// @param cmd Имя команды
	/// @param iss Поток ввода с аргументами команды
	/// @return Указатель на созданную команду или nullptr если команда не распознана
	std::unique_ptr<ICommand> createCommand(const std::string& cmd, std::istringstream& iss) {
		if (cmd.empty() || cmd[0] == '#') {
			return nullptr; // Пропускаем пустые строки и комментарии
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

/// @brief Обрабатывает команды из входного потока
/// @param input Входной поток с командами
/// @param interactive Флаг интерактивного режима (по умолчанию true)
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