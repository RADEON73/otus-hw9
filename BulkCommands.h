#pragma once
#include "BulkProcessor.h"

/**
 * @class IBulkCommand
 * @brief Интерфейс для всех команд.
 *
 * Интерфейс IBulkCommand определяет метод execute, который должен быть реализован
 * всеми классами команд.
 */
class IBulkCommand
{
public:
	virtual ~IBulkCommand() = default;
	/**
	* @brief Выполняет команду.
	* @param processor Объект BulkProcessor, который будет обрабатывать команду.
	*/
	virtual void execute(BulkProcessor& processor) = 0;
};

/**
 * @class StartBlockCommand
 * @brief Команда для начала нового динамического блока.
 *
 * Класс StartBlockCommand реализует команду начала нового динамического блока.
 */
class StartBlockCommand : public IBulkCommand
{
public:
	void execute(BulkProcessor& processor) override {
		processor.startBlock();
	}
};

/**
 * @class EndBlockCommand
 * @brief Команда для завершения текущего динамического блока.
 *
 * Класс EndBlockCommand реализует команду завершения текущего динамического блока.
 */
class EndBlockCommand : public IBulkCommand
{
public:
	void execute(BulkProcessor& processor) override {
		processor.endBlock();
	}
};

/**
 * @class RegularCommand
 * @brief Команда для добавления обычной команды в блок.
 *
 * Класс RegularCommand реализует команду добавления обычной команды в текущий блок.
 */
class RegularCommand : public IBulkCommand
{
public:
	/**
 * @brief Конструктор класса RegularCommand.
 * @param command Команда для добавления.
 */
	explicit RegularCommand(const std::string& command) : command_(command) {}

	void execute(BulkProcessor& processor) override {
		processor.addCommand(command_);
	}

private:
	std::string command_; ///< Команда для добавления.
};