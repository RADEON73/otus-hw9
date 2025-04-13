/**
 * @file MultiThreadOutputter.cpp
 * @brief Реализация класса MultiThreadOutputter
 */
#include "MultiThreadOutputter.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <stop_token>

MultiThreadOutputter::~MultiThreadOutputter()
{
	stop_source_.request_stop(); // Посылаем сигнал остановки
}

MultiThreadOutputter& MultiThreadOutputter::getInstance() {
	static MultiThreadOutputter instance;
	return instance;
}

void MultiThreadOutputter::request_stop()
{
	if (!stop_source_.stop_requested()) {
		stop_source_.request_stop();
		// Ожидаем завершения с таймаутом
		auto start = std::chrono::steady_clock::now();
		while (!log_queue.empty() || !file_queue.empty()) {
			if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
				std::cerr << "Warning: Force stopping with pending items\n";
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
}

MultiThreadOutputter::MultiThreadOutputter() :
	log_thread(&MultiThreadOutputter::log_worker, this, stop_source_.get_token()),
	file_thread1(&MultiThreadOutputter::file_worker, this, 1, stop_source_.get_token()),
	file_thread2(&MultiThreadOutputter::file_worker, this, 2, stop_source_.get_token())
{
}

void MultiThreadOutputter::log_worker(std::stop_token stoken) {
	ITEM item;
	while (!stoken.stop_requested()) {
		if (log_queue.try_pop(item))
			process_log_item(item);
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Опционально: снижаем нагрузку на CPU
	}
	while (log_queue.try_pop(item))
		process_log_item(item);
}

void MultiThreadOutputter::process_log_item(const ITEM& item) const
{
	auto& [commands, timestamp] = item;

	std::cout << "bulk: ";
	for (size_t i = 0; i < commands.size(); ++i) {
		std::cout << commands[i];
		if (i < commands.size() - 1)
			std::cout << ", ";
	}
	std::cout << std::endl;
}

void MultiThreadOutputter::process_file_item(int id, const ITEM& item, std::mt19937& gen, std::uniform_int_distribution<>& dis) const
{
	auto& [commands, timestamp] = item;

	std::filesystem::path logDir = "LOG";
	if (!std::filesystem::exists(logDir)) {
		std::filesystem::create_directory(logDir);
	}
	std::stringstream filename;
	filename << "bulk" << timestamp << "_threadID_" << id << "_" << dis(gen) << ".log";
	std::filesystem::path filePath = logDir / filename.str();
	std::ofstream file;
	file.rdbuf()->pubsetbuf(nullptr, 0); // Отключаем буферизацию
	file.open(filePath, std::ios::app);
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

void MultiThreadOutputter::file_worker(int id, std::stop_token stoken) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution dis(100000000, 999999999);
	ITEM item;
	while (!stoken.stop_requested()) {

		if (file_queue.try_pop(item)) {
			process_file_item(id, item, gen, dis);
		}
		else if (log_queue.size() > 100) { // Балансировка нагрузки: если log_queue переполнен, то помогаем с обработкой
			if (log_queue.try_pop(item))  
				process_log_item(item);
		}
		else
			std::this_thread::yield();
	}

	while (file_queue.try_pop(item)) {
		process_file_item(id, item, gen, dis);
	}
}