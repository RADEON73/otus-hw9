#pragma once
#include "async.h"
#include <unordered_map>
#include <mutex>
#include <atomic>

/**
 * @class ProcessorManager
 * @brief Менеджер для управления процессорами команд
 * @details Обеспечивает потокобезопасное управление коллекцией процессоров,
 *          включая их добавление, удаление и поиск.
 */
class ProcessorManager
{
private:
	std::unordered_map<async::HANDLE, int> processors;  ///< Хранилище процессоров (хэндл → ID)
	int next_id = 1;                                    ///< Счетчик для генерации ID
	std::atomic<bool> closeRequest_{ false };           ///< Флаг запроса на завершение
	mutable std::mutex mutex_;                           ///< Мьютекс для синхронизации доступа

public:
	/**
	 * @brief Добавляет новый процессор в менеджер
	 * @param handle Хэндл процессора для добавления
	 * @return ID присвоенный процессору
	 * @note Потокобезопасный метод
	 */
	int addProcessor(async::HANDLE handle)
	{
		std::lock_guard lock(mutex_);
		processors[handle] = next_id++;
		return next_id - 1;
	}

	/**
	 * @brief Удаляет процессор по его ID
	 * @param id ID процессора для удаления
	 * @return true если процессор был найден и удален, false в противном случае
	 * @note Потокобезопасный метод
	 */
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

	/**
	 * @brief Получает хэндл процессора по его ID
	 * @param id ID процессора для поиска
	 * @return Хэндл процессора или nullptr если не найден
	 * @note Потокобезопасный метод
	 */
	async::HANDLE getProcessor(int id) const
	{
		std::lock_guard lock(mutex_);
		for (const auto& [h, i] : processors) {
			if (i == id)
				return h;
		}
		return nullptr;
	}

	/**
	 * @brief Возвращает все зарегистрированные процессоры
	 * @return Константная ссылка на unordered_map с процессорами
	 * @note Потокобезопасный метод
	 */
	const std::unordered_map<async::HANDLE, int>& getAllProcessors() const
	{
		std::lock_guard lock(mutex_);
		return processors;
	}

	/**
	 * @brief Проверяет отсутствие зарегистрированных процессоров
	 * @return true если нет процессоров, false в противном случае
	 * @note Потокобезопасный метод
	 */
	bool empty() const {
		std::lock_guard lock(mutex_);
		return processors.empty();
	}

	/**
	 * @brief Устанавливает флаг запроса на завершение работы
	 * @note Использует атомарную операцию для установки флага
	 */
	void closeRequest() {
		closeRequest_.store(true, std::memory_order_release);
	}

	/**
	 * @brief Проверяет наличие запроса на завершение работы
	 * @return true если был запрос на завершение, false в противном случае
	 * @note Использует атомарную операцию для чтения флага
	 */
	bool isCloseRequested() const {
		return closeRequest_.load(std::memory_order_acquire);
	}
};