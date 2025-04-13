/**
 * @file ThreadSafeQueue.h
 * @brief Потокобезопасная очередь
 * @tparam T Тип элементов очереди
 */

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue
{
	std::queue<T> queue_;
	mutable std::mutex mutex_;
	std::condition_variable cond_;

public:
	/**
	* @brief Добавляет элемент в очередь
	* @param item Элемент для добавления
	*/
	void push(T item) {
		std::scoped_lock lock(mutex_);
		queue_.push(std::move(item));
		cond_.notify_one();
	}

	/**
	* @brief Проверяет пустоту очереди
	* @return true если очередь пуста, иначе false
	*/
	bool empty() const {
		std::scoped_lock lock(mutex_);
		return queue_.empty();
	}

	/**
	* @brief Извлекает элемент из очереди с ожиданием
	* @param item Ссылка для сохранения извлеченного элемента
	*/
	void wait_and_pop(T& item) {
		std::unique_lock lock(mutex_);
		cond_.wait(lock, [this] { return !queue_.empty(); });
		item = std::move(queue_.front());
		queue_.pop();
	}

	bool try_pop(T& item) {
		std::scoped_lock lock(mutex_);
		if (queue_.empty()) return false;
		item = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	size_t size() const {
		std::scoped_lock lock(mutex_);
		return queue_.size();
	}
};