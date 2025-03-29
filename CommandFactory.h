#pragma once
#include <memory>
#include "Commands.h"

/**
 * @class CommandFactory
 * @brief Фабрика для создания команд.
 *
 * Класс CommandFactory создает объекты команд на основе входной строки.
 */
class CommandFactory
{
public:
	/**
 * @brief Создает команду на основе входной строки.
 * @param command Входная строка команды.
 * @return Указатель на созданную команду.
 */
	static std::unique_ptr<ICommand> create(const std::string& command) {
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