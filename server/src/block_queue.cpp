#include "block_queue.hpp"

BlockQueue::BlockQueue(std::weak_ptr<FileStorage> file_storage) :
	done(false),
	file_storage(file_storage)
{
	persist_thread = std::thread(&BlockQueue::run, this);
}

BlockQueue::~BlockQueue()
{
	stop();
}

void BlockQueue::stop()
{
	flush(std::unique_lock<std::mutex>(block_mtx));

	done = true;
	if (persist_thread.joinable())
		persist_thread.join();
}

bool BlockQueue::lookup(key_type key, std::vector<value_type>& result) const
{
	bool found = false;
	if (!found)
	{
		std::unique_lock<std::mutex> lock(block_mtx);
		found = lookup(block_map, key, result);
	}

	if (!found)
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		for (auto& m : block_queue)
		{
			found = lookup(m, key, result);
			if (found)
				break;
		}
	}
	return found;
}

void BlockQueue::enqueue(key_type& key, value_type& value)
{
	std::unique_lock<std::mutex> block_lock(block_mtx);
	block_map.emplace(key, value);

	if (block_map.size() == MAX_BLOCK_SIZE)
	{
		flush(std::move(block_lock));
	}
}

void BlockQueue::run()
{
	while (!done)
	{
		std::unique_lock<std::mutex> lock(block_mtx);
		block_ready.wait(lock, [this]() { return done || !block_queue.empty(); });

		while (!block_queue.empty())
		{
			data_map_type block = std::move(block_queue.front());
			block_queue.pop_front();

			lock.unlock();

			if (!block.empty() && !file_storage.expired())
				file_storage.lock()->put_data(std::move(block));

			lock.lock();
		}
	}
}


void BlockQueue::flush(std::unique_lock<std::mutex>&& block_lock)
{
	data_map_type block;
	block.swap(block_map);
	block_lock.unlock();

	{
		std::unique_lock<std::mutex> queue_lock(queue_mtx);
		block_queue.emplace_back(std::move(block));
	}

	block_ready.notify_one();
}

bool BlockQueue::lookup(const data_map_type& m, key_type key, std::vector<value_type>& result)
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