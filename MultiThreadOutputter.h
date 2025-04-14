#pragma once
#include "ThreadSafeQueue.h"
#include <mutex>
#include <string>
#include <thread>
#include <stop_token>
#include <random>

class MultiThreadOutputter
{
	using ITEM = std::pair<std::vector<std::string>, time_t>;

public:
	MultiThreadOutputter(const MultiThreadOutputter&) = delete;
	MultiThreadOutputter& operator=(const MultiThreadOutputter&) = delete;

	~MultiThreadOutputter();

	static MultiThreadOutputter& getInstance();

	ThreadSafeQueue<ITEM> log_queue; ///< Очередь логирования
	ThreadSafeQueue<ITEM> file_queue; ///< Очередь записи в файл

private:
	MultiThreadOutputter();

	std::jthread log_thread; ///< Поток логирования
	std::jthread file_thread1; ///< Поток записи в файл 1
	std::jthread file_thread2; ///< Поток записи в файл 2
	std::stop_source stop_source_; // Источник сигнала остановки

	/**
	* @brief Рабочая функция потока логирования
	*
	* Обрабатывает команды из очереди и выводит их в консоль
	*/
	void log_worker(std::stop_token stoken);

	/**
	* @brief Рабочая функция потока записи в файл
	* @param id Идентификатор потока (используется в имени файла)
	*
	* Обрабатывает команды из очереди и записывает их в файл
	*/
	void file_worker(int id, std::stop_token stoken);

	void process_log_item(const ITEM& item) const;
	void process_file_item(int id, const ITEM& item, std::mt19937& gen, std::uniform_int_distribution<>& dis) const;
};