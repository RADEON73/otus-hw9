#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

/**
 * @class BulkProcessor
 * @brief Обрабатывает команды и управляет блоками команд.
 *
 * Класс BulkProcessor предназначен для обработки команд, группировки их в блоки
 * и выполнения операций над этими блоками, таких как вывод на экран и логирование.
 */
class BulkProcessor
{
public:
	/**
	* @brief Конструктор класса BulkProcessor.
	* @param block_size Размер блока команд.
	*/
	explicit BulkProcessor(size_t block_size);

	/**
	* @brief Деструктор класса BulkProcessor.
	*/
	~BulkProcessor();

	/**
	* @brief Завершает обработку команд и выполняет финализацию.
	*/
	void finalize();

	/**
	* @brief Начинает новый динамический блок команд.
	*/
	void startBlock();

	/**
	* @brief Завершает текущий динамический блок команд.
	*/
	void endBlock();

	/**
	* @brief Добавляет команду в текущий блок.
	* @param command Команда для добавления.
	*/
	void addCommand(const std::string& command);

	/**
	* @brief Обрабатывает входную строку команд.
	* @param input Входная строка команд.
	*/
	void parse(const std::string_view& input);

private:
	/**
	* @brief Обрабатывает команду.
	* @param command Команда для обработки.
	*/
	void process(const std::string& command);
	/**
	* @brief Сбрасывает текущий блок, выводя и логируя его содержимое.
	*/
	void flush();

	/**
	* @struct Block
	* @brief Структура, представляющая блок команд.
	*/
	struct Block
	{
		/**
		* @brief Сбрасывает блок, очищая его данные и сбрасывая флаги.
		*/
		void reset();

		std::vector<std::string> data; ///< Вектор команд в блоке.
		bool is_dynamic{ false }; ///< Флаг, указывающий, является ли блок динамическим.
		size_t depth{ 0 }; ///< Глубина вложенности блоков.
		time_t createTimeStamp{ 0 }; ///< Время создания блока
	};

	size_t block_size_; ///< Размер блока команд.
	Block current_block_; ///< Текущий блок команд.
	mutable std::mutex mutex_;
};