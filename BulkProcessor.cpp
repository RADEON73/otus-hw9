#include "BulkProcessor.h"
#include "CommandFactory.h"
#include <chrono>
#include <string>
#include "MultiThreadOutputter.h"

BulkProcessor::BulkProcessor(size_t block_size) : block_size_(block_size)
{
	MultiThreadOutputter::getInstance().client_count++;
}

BulkProcessor::~BulkProcessor()
{
	MultiThreadOutputter::getInstance().client_count--;
	if (MultiThreadOutputter::getInstance().client_count <= 0)
		MultiThreadOutputter::getInstance().waitUntilDone();
}

void BulkProcessor::startBlock() {
	if (current_block_.depth == 0)
		flush();
	++current_block_.depth;
	current_block_.is_dynamic = true;
}

void BulkProcessor::endBlock() {
	if (current_block_.depth > 0) {
		--current_block_.depth;
		if (current_block_.depth == 0)
			flush();
	}
}

void BulkProcessor::addCommand(const std::string& command) {
	if (current_block_.data.empty())
		current_block_.createTimeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	current_block_.data.push_back(command);
	if (!current_block_.is_dynamic && current_block_.data.size() >= block_size_)
		flush();
}

void BulkProcessor::parse(const std::string_view& input)
{
	size_t start = 0;
	size_t end = input.find('\n');
	while (end != std::string::npos) {
		if (std::string command(input.substr(start, end - start)); !command.empty())
			process(command);
		start = end + 1;
		end = input.find('\n', start);
	}
	if (start < input.size()) {
		std::string command(input.substr(start));
		if (!command.empty()) {
			process(command);
		}
	}
}

void BulkProcessor::process(const std::string& command) {
	auto cmd = CommandFactory::create(command);
	cmd->execute(*this);
}

void BulkProcessor::finalize() {
	if (current_block_.depth == 0)
		flush();
	else
		current_block_.data.clear();
}

void BulkProcessor::Block::reset()
{
	data.clear();
	is_dynamic = false;
	depth = 0;
	createTimeStamp = 0;
}

void BulkProcessor::flush() {
	if (!current_block_.data.empty()) {
		auto& outputter = MultiThreadOutputter::getInstance();
		outputter.log_queue.push({ current_block_.data, current_block_.createTimeStamp });
		outputter.file_queue.push({ current_block_.data, current_block_.createTimeStamp });
		current_block_.reset();
	}
}