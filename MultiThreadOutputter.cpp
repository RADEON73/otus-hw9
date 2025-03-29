/**
 * @file MultiThreadOutputter.cpp
 * @brief Реализация класса MultiThreadOutputter
 */
#include "MultiThreadOutputter.h"
#include <iostream>
#include <filesystem>
#include <fstream>

MultiThreadOutputter::~MultiThreadOutputter()
{
	if (log_thread.joinable())
		log_thread.join();
	if (file_thread1.joinable())
		file_thread1.join();
	if (file_thread2.joinable())
		file_thread2.join();
}

MultiThreadOutputter& MultiThreadOutputter::getInstance() {
	static MultiThreadOutputter instance;
	return instance;
}

MultiThreadOutputter::MultiThreadOutputter()
{
	log_thread = std::jthread([this] { this->log_worker(); });
	file_thread1 = std::jthread([this] { this->file_worker(1); });
	file_thread2 = std::jthread([this] { this->file_worker(2); });
}

void MultiThreadOutputter::log_worker() {
	while (client_count || !log_queue.empty()) {
		std::pair<std::vector<std::string>, time_t> item;
		log_queue.wait_and_pop(item);
		auto& [commands, timestamp] = item;

		std::cout << "bulk: ";
		for (size_t i = 0; i < commands.size(); ++i) {
			std::cout << commands[i];
			if (i < commands.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;
	}
}

void MultiThreadOutputter::file_worker(int id) {
	while (client_count || !file_queue.empty()) {
		std::pair<std::vector<std::string>, time_t> item;
		file_queue.wait_and_pop(item);
		auto& [commands, timestamp] = item;

		std::filesystem::path logDir = "LOG";
		if (!std::filesystem::exists(logDir)) {
			std::filesystem::create_directory(logDir);
		}
		std::stringstream filename;
		filename << "bulk" << timestamp << "_threadID_" << id << ".log";
		std::filesystem::path filePath = logDir / filename.str();
		std::ofstream file(filePath, std::ios::app);
		if (!file.is_open()) {
			std::cerr << "Error opening file: " << filePath << std::endl;
			return;
		}
		file << "bulk: ";
		for (size_t i = 0; i < commands.size(); ++i) {
			file << commands[i];
			if (i < commands.size() - 1)
				file << ", ";
		}
		file << std::endl;
	}
}

void MultiThreadOutputter::waitUntilDone() const
{
	// Ждем, пока все очереди не опустеют и все клиенты не отключатся
	while (client_count > 0 || !log_queue.empty() || !file_queue.empty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Дополнительная проверка на всякий случай
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
