/**
 * @file main.cpp
 * @brief Основной файл демонстрации работы асинхронной обработки команд
 */
#include "async.h"
#include <string>

/**
 * @brief Основная функция программы
 * @return Код возврата (0 - успешное выполнение)
 *
 * Создает два процессора для обработки команд, отправляет тестовые данные
 * и корректно завершает работу с освобождением ресурсов.
 */

int main() {
	using namespace async;

	auto packSize = 3; ///Размер пакета

	auto bp1 = connect(packSize); ///Первый процессор
	auto bp2 = connect(packSize); ///Второй процессор

	std::string buffer11 = "stat1\nstat2\nstat3\n"; ///Тестовые данные первому процессору (пакет 1)
	std::string buffer12 = "stat4\nstat5\n"; ///Тестовые данные первому процессору (пакет 2)
	std::string buffer21 = "cmd1\ncmd2\n{\ncmd3\ncmd4\n}\n"; ///Тестовые данные второму процессору (пакет 1)
	std::string buffer22 = "{\ncmd5\ncmd6\n{\ncmd7\ncmd8\n}\ncmd9\n}\n{\ncmd10\ncmd11\n"; ///Тестовые данные второму процессору (пакет 2)

	receive(bp1, buffer11.data(), buffer11.size()); ///Отправляем первому процессору (пакет 1)
	receive(bp2, buffer21.data(), buffer21.size()); ///Отправляем второму процессору (пакет 1)
	receive(bp1, buffer12.data(), buffer12.size()); ///Отправляем первому процессору (пакет 2)
	receive(bp2, buffer22.data(), buffer22.size()); ///Отправляем второму процессору (пакет 2)

	disconnect(bp1); ///Отключаем первый процессор
	disconnect(bp2); ///Отключаем второй процессор

	return 0;
}