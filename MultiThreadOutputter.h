#pragma once
#include "ThreadSafeQueue.h"
#include <mutex>
#include <string>

class MultiThreadOutputter
{
public:
	MultiThreadOutputter(const MultiThreadOutputter&) = delete;
	MultiThreadOutputter& operator=(const MultiThreadOutputter&) = delete;

	~MultiThreadOutputter();

	/**
	* @brief Ожидает завершения работы потоков
	*
	* Не дает завершить приложение до тех пор, пока очереди не опустеют, а потоки не завершатся
	*/
	void waitUntilDone() const;

	static MultiThreadOutputter& getInstance();

	ThreadSafeQueue<std::pair<std::vector<std::string>, time_t>> log_queue; ///< Очередь логирования
	ThreadSafeQueue<std::pair<std::vector<std::string>, time_t>> file_queue; ///< Очередь записи в файл

	std::atomic<int> client_count; ///< Количество клиентов (лоя подсчета завершения процесса)
	std::atomic<bool> running{ true };  ///< Флаг для контроля работы потоков

private:
	MultiThreadOutputter();

	std::jthread log_thread; ///< Поток логирования
	std::jthread file_thread1; ///< Поток записи в файл 1
	std::jthread file_thread2; ///< Поток записи в файл 2

	/**
	* @brief Рабочая функция потока логирования
	*
	* Обрабатывает команды из очереди и выводит их в консоль
	*/
	void log_worker();

	/**
	* @brief Рабочая функция потока записи в файл
	* @param id Идентификатор потока (используется в имени файла)
	*
	* Обрабатывает команды из очереди и записывает их в файл
	*/
	void file_worker(int id);
};