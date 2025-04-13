#pragma once
#include <memory>
#include "BulkCommands.h"

/**
 * @class BulkCommandFactory
 * @brief Фабрика для создания команд.
 *
 * Класс BulkCommandFactory создает объекты команд на основе входной строки.
 */
class BulkCommandFactory
{
public:
	/**
 * @brief Создает команду на основе входной строки.
 * @param command Входная строка команды.
 * @return Указатель на созданную команду.
 */
	static std::unique_ptr<IBulkCommand> create(const std::string& command) {
		if (command == "{") {
			return std::make_unique<StartBlockCommand>();
		}
		else if (command == "}") {
			return std::make_unique<EndBlockCommand>();
		}
		else {
			return std::make_unique<RegularCommand>(command);
		}
	}
};