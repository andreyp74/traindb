#ifndef STORAGE_HPP_
#define STORAGE_HPP_

#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <deque>

#include "file_storage.hpp"

class Storage
{
public:
	Storage() :
		persist_done(false),
		block_size(0)
	{
		file_storage = std::make_unique<FileStorage>("./.local/");
		persist_thread = std::thread(&Storage::persist_run, this);
	}

	~Storage()
	{
		flush(std::unique_lock<std::mutex>(data_mtx));

		persist_done = true;
		block_ready.notify_one();
		if (persist_thread.joinable())
			persist_thread.join();
	}

	void put_data(key_type key, value_type value)
	{
		std::unique_lock<std::mutex> lock(data_mtx);
		data.emplace(key, value);

		if (++block_size == MAX_BLOCK_SIZE)
		{
			block_size = 0;
			flush(std::move(lock));
		}
	}

	std::vector<value_type> get_data(key_type key) const
	{
		std::vector<value_type> result;
		bool found = true;

		{
			std::unique_lock<std::mutex> lock(data_mtx);
			found = lookup(data, key, result);
		}

		if (!found)
		{
			std::unique_lock<std::mutex> lock(block_mtx);
			for (auto& m : block_queue)
			{
				found = lookup(m, key, result);
				if (found)
					break;
			}
		}

		if (!found)
			file_storage->get_data(key, result);

		return result;
	}

	void flush(std::unique_lock<std::mutex>&& lock)
	{
		data_map_type block;
		block.swap(data);
		lock.unlock();

		{
			std::unique_lock<std::mutex> block_lock(block_mtx);
			block_queue.push_back(block);
		}
		block_ready.notify_one();
	}

protected:

	void persist_run()
	{
		while (!persist_done)
		{
			std::unique_lock<std::mutex> lock(block_mtx);
			block_ready.wait(lock, [this]() { return persist_done || !block_queue.empty(); });

			while (!block_queue.empty())
			{
				auto block = block_queue.front();
				block_queue.pop_front();

				lock.unlock();

				if (!block.empty())
					file_storage->put_data(std::move(block));

				lock.lock();
			}
		}
	}

private:
	Storage(Storage&) = delete;
	Storage& operator=(Storage&) = delete;


	static bool lookup(const data_map_type& m, key_type key, std::vector<value_type>& result)
	{
		bool found = false;
		auto range = m.equal_range(key);

		if (range.first != range.second)
		{
			for (auto it = range.first; it != range.second; ++it)
				result.push_back(it->second);
			found = true;
		}

		return found;
	}

private:

	size_t MAX_BLOCK_SIZE = 1024;

	data_map_type data;
	mutable std::mutex data_mtx;

	std::unique_ptr<FileStorage> file_storage;

	std::atomic<size_t> block_size;
	std::condition_variable block_ready;
	mutable std::mutex block_mtx;
	std::deque<data_map_type> block_queue;

	std::thread persist_thread;
	std::atomic<bool> persist_done;
};

#endif //STORAGE_HPP_