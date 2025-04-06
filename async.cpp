/**
 * @file async.cpp
 * @brief Реализация функций асинхронного интерфейса
 */

#include "async.h"
#include "BulkProcessor.h"
#include <string>

namespace async {
	HANDLE connect(size_t packSize) {
		return new BulkProcessor(packSize);
	}

	void receive(HANDLE handle, const char* data, size_t size) {
		auto processor = static_cast<BulkProcessor*>(handle);
		std::string input(data, size);
		processor->parse(input);
	}

	void disconnect(HANDLE handle) {
		auto processor = static_cast<BulkProcessor*>(handle);
		processor->finalize();
		delete processor;
	}
}