/**
* @file main.cpp
* @brief Интерактивный клиент для работы с асинхронной обработкой команд
*/

#define V2_DEMO_PROJECT

#include <iostream>
#include <fstream>
#include "ProcessorCommands.h"
#include <iosfwd>
#include <thread>
#include <vector>
#include "async.h"

namespace RESULT {
	enum CODE
	{
		OK,
		ARGUMENT_PARSE_ERROR,
		FILE_OPENING_ERROR
	};
}

/// Stress test - 10 threads send 1000 commands  - лучше не запускать :)
void stress_test() {
	auto handle1 = async::connect(3);
	auto handle2 = async::connect(2);

	std::vector<std::jthread> threads;
	for (int i = 0; i < 10; ++i) {
		threads.emplace_back([handle1, handle2] {
			for (int j = 0; j < 1000; ++j) {
				async::receive(handle1, "cmd\n", 4);
				async::receive(handle2, "cmd\n", 4);
			}
			});
	}

	for (auto& t : threads) t.join();

	async::disconnect(handle1);
	async::disconnect(handle2);
}

int main(int argc, char* argv[]) {
#ifdef V2_DEMO_PROJECT
	if (argc != 2) {
		std::cerr << "Usage: async <bulk_size>\n";
		return RESULT::ARGUMENT_PARSE_ERROR;
	}
	ProcessorManager manager;
	CommandFactory factory(manager, true);

	auto handle = async::connect(atoi(argv[1]));
	manager.addProcessor(handle);

	std::string line;
	while (getline(std::cin, line)) {
		std::istringstream iss(line);
		std::string cmd;
		iss >> cmd;
		if (auto command = factory.createCommand(cmd, iss))
			command->execute();
		if (manager.isCloseRequested())
			break;
	}

	async::disconnect(handle);

	return RESULT::OK;
#else
	std::ifstream input_file;
	switch (argc) {
	case 1: // Interactive mode
		process_commands_from_stream(std::cin, true);
		break;
	case 2: // File mode
		input_file.open(argv[1]);
		if (!input_file.is_open()) {
			std::cerr << "Error: cannot open file '" << argv[1] << "'\n";
			return RESULT::FILE_OPENING_ERROR;
		}
		process_commands_from_stream(input_file, false);
		break;
	default:
		std::cerr << "Too many arguments " << std::endl;
		return RESULT::ARGUMENT_PARSE_ERROR;
	}
	return RESULT::OK;
#endif
}

/*
stat1\nstat2\nstat3\nstat4\nstat5\ncmd1\ncmd2\n{\ncmd3\ncmd4\n}\n{\ncmd5\ncmd6\n{\ncmd7\ncmd8\n}\ncmd9\n}\n{\ncmd10\ncmd11\n
*/