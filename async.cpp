/**
 * @file async.cpp
 * @brief Реализация функций асинхронного интерфейса
 */

#include "async.h"
#include "BulkProcessor.h"
#include <string>
#include <iostream>
#include "MultiThreadOutputter.h"
#include <thread>

namespace async {
	HANDLE connect(size_t packSize) {
		return new BulkProcessor(packSize);
	}

	void receive(HANDLE handle, const char* data, size_t size) {
		if (!handle || !data || size == 0) {
			throw std::invalid_argument("Invalid arguments in receive()");
		}
		try {
			auto processor = static_cast<BulkProcessor*>(handle);
			std::string input(data, size);
			processor->parse(input);
		}
		catch (const std::exception& e) {
			std::cerr << "Error in receive(): " << e.what() << std::endl;
			throw;
		}
	}

	void disconnect(HANDLE handle) {
		if (!handle) 
			return;
		auto processor = static_cast<BulkProcessor*>(handle);
		processor->finalize();
		while (!MultiThreadOutputter::getInstance().log_queue.empty() || !MultiThreadOutputter::getInstance().file_queue.empty())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		delete processor;
	}
}