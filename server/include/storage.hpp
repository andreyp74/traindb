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

#include "version_list.hpp"
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
		//flush(std::unique_lock<std::mutex>(data_mtx));

		persist_done = true;
		block_ready.notify_one();
		if (persist_thread.joinable())
			persist_thread.join();
	}

	bool commit(key_type key, int version)
	{
		bool is_committed;
		value_type value;
		{
			std::unique_lock<std::mutex> lock(data_mtx);

			auto it = data.find(key);
			if (it == data.end())
				return false;

			is_committed = it->second.commit(version, value);
		}
		if (is_committed)
			enqueue(key, value);

		return is_committed;
	}

	int put_data(key_type key, value_type&& value)
	{
		std::unique_lock<std::mutex> lock(data_mtx);

		VersionList& ls = data[key];
		return ls.add_version(std::move(value));
	}

	value_type get_data(key_type key) const
	{
		value_type result;
		bool found = true;

		{
			std::unique_lock<std::mutex> lock(data_mtx);
			found = lookup(key, result);
		}

		if (!found)
		{
			std::unique_lock<std::mutex> lock(block_mtx);
			auto it = block_map.find(key);
			if (it != block_map.end())
			{
				result = it->second;
				found = true;
			}
		}

		if (!found)
		{
			std::unique_lock<std::mutex> lock(queue_mtx);
			found = lookup_in_queue(key, result);
		}

		if (!found)
		{
			std::vector<value_type> result_vec;
			file_storage->get_data(key, result_vec);
			if (!result_vec.empty())
				result = result_vec.back();
		}

		return result;
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


	bool lookup(key_type key, value_type& value) const
	{
		auto it = data.find(key);
		if (it != data.end())
		{
			if (it->second.get_last(value))
				return true;
		}
		return false;
	}

	bool lookup_in_queue(key_type key, value_type& value) const
	{
		for (auto& m : block_queue)
		{
			auto it = m.find(key);
			if (it != m.end())
			{
				value = it->second;
				return true;
			}
		}
		return false;
	}

	void enqueue(key_type& key, value_type& value)
	{
		std::unique_lock<std::mutex> block_lock(block_mtx);
		block_map.emplace(key, value);

		if (++block_size == MAX_BLOCK_SIZE)
		{
			data_map_type block;
			block.swap(block_map);
			block_lock.unlock();

			{
				std::unique_lock<std::mutex> queue_lock(queue_mtx);
				block_queue.push_back(block);
			}

			block_size = 0;
			block_ready.notify_one();
		}
	}

private:

	size_t MAX_BLOCK_SIZE = 1024;

	data_map_ver_type data;
	mutable std::mutex data_mtx;

	std::unique_ptr<FileStorage> file_storage;

	mutable std::mutex block_mtx;
	data_map_type block_map;

	std::atomic<size_t> block_size;
	std::condition_variable block_ready;
	mutable std::mutex queue_mtx;
	std::deque<data_map_type> block_queue;

	std::thread persist_thread;
	std::atomic<bool> persist_done;
};

#endif //STORAGE_HPP_