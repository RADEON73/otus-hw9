#pragma once
#include "async.h"
#include <unordered_map>
#include <mutex>

/// ����� ��� ���������� ������������
class ProcessorManager
{
private:
	std::unordered_map<async::HANDLE, int> processors;
	int next_id = 1;
	std::atomic<bool> closeRequest_{ false };
	mutable std::mutex mutex_;

public:
	/// ���������� ������ ����������
	int addProcessor(async::HANDLE handle)
	{
		std::lock_guard lock(mutex_);
		processors[handle] = next_id++;
		return next_id - 1;
	}

	/// �������� ���������� �� ID
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

	/// ��������� ���������� �� ID
	async::HANDLE getProcessor(int id) const
	{
		std::lock_guard lock(mutex_);
		for (const auto& [h, i] : processors) {
			if (i == id)
				return h;
		}
		return nullptr;
	}

	/// ��������� ������ ���� �����������
	const std::unordered_map<async::HANDLE, int>& getAllProcessors() const
	{
		std::lock_guard lock(mutex_);
		return processors;
	}

	/// �������� �� �������
	bool empty() const { 
		std::lock_guard lock(mutex_);
		return processors.empty(); 
	}

	/// ������ �� ����������
	void closeRequest() { 
		closeRequest_.store(true, std::memory_order_release);
	}

	/// �������� �� ������ ����������
	bool isCloseRequested() const { 
		return closeRequest_.load(std::memory_order_acquire);
	}
};