#pragma once
#include "async.h"
#include <unordered_map>
#include <mutex>

/// Класс для управления процессорами
class ProcessorManager
{
private:
	std::unordered_map<async::HANDLE, int> processors;
	int next_id = 1;
	std::atomic<bool> closeRequest_{ false };
	mutable std::mutex mutex_;

public:
	/// Добавление нового процессора
	int addProcessor(async::HANDLE handle)
	{
		std::lock_guard lock(mutex_);
		processors[handle] = next_id++;
		return next_id - 1;
	}

	/// Удаление процессора по ID
	bool removeProcessor(int id)
	{
		std::lock_guard lock(mutex_);
		for (auto it = processors.begin(); it != processors.end(); ++it) {
			if (it->second == id) {
				processors.erase(it);
				return true;
			}
		}
		return false;
	}

	/// Получение процессора по ID
	async::HANDLE getProcessor(int id) const
	{
		std::lock_guard lock(mutex_);
		for (const auto& [h, i] : processors) {
			if (i == id)
				return h;
		}
		return nullptr;
	}

	/// Получение списка всех процессоров
	const std::unordered_map<async::HANDLE, int>& getAllProcessors() const
	{
		std::lock_guard lock(mutex_);
		return processors;
	}

	/// Проверка на пустоту
	bool empty() const { 
		std::lock_guard lock(mutex_);
		return processors.empty(); 
	}

	/// Запрос на завершение
	void closeRequest() { 
		closeRequest_.store(true, std::memory_order_release);
	}

	/// Проверка на запрос завершения
	bool isCloseRequested() const { 
		return closeRequest_.load(std::memory_order_acquire);
	}
};