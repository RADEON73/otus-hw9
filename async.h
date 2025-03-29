/**
 * @file async.h
 * @brief Заголовочный файл асинхронного интерфейса
 */

#pragma once

namespace async {

	/**
	* @brief Указатель на процессор команд
	*/
	using HANDLE = void*;

	/**
	* @brief Создает новый процессор команд
	* @param packSize Размер блока команд
	* @return Указатель на созданный процессор
	*/
	HANDLE connect(size_t packSize);

	/**
	 * @brief Передает данные для обработки
	 * @param handle Указатель на процессор
	 * @param data Указатель на данные
	 * @param size Размер данных
	 */
	void receive(HANDLE handle, const char* data, size_t size);

	/**
	 * @brief Завершает работу процессора
	 * @param handle Указатель на процессор
	 */
	void disconnect(HANDLE handle);
}