/**
 * @file async.cpp
 * @brief Реализация функций асинхронного интерфейса
 */

#include "async.h"
#include "BulkProcessor.h"
#include <string>
#include <bit>

namespace async {
	HANDLE connect(size_t packSize) {
		return new BulkProcessor(packSize);
	}

	void receive(HANDLE handle, const char* data, size_t size) {
		auto processor = std::bit_cast<BulkProcessor*>(handle);
		std::string input(data, size);
		processor->parse(input);
	}

	void disconnect(HANDLE handle) {
		auto processor = std::bit_cast<BulkProcessor*>(handle);
		processor->finalize();
		delete processor;
	}
}