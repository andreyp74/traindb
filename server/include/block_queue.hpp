#ifndef BLOCK_QUEUE_HPP_
#define BLOCK_QUEUE_HPP_

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <deque>

#include "file_storage.hpp"

class BlockQueue
{
public:
	BlockQueue(std::weak_ptr<FileStorage> file_storage);
	~BlockQueue();

	void stop();

	bool lookup(key_type key, std::vector<value_type>& result) const;
	void enqueue(key_type& key, value_type& value);

private:

	BlockQueue(BlockQueue&) = delete;
	BlockQueue& operator=(BlockQueue&) = delete;

	void run();

	void flush(std::unique_lock<std::mutex>&& block_lock);

	static bool lookup(const data_map_type& m, key_type key, std::vector<value_type>& result);

private:
	size_t MAX_BLOCK_SIZE = 1024;

	std::weak_ptr<FileStorage> file_storage;

	mutable std::mutex block_mtx;
	data_map_type block_map;

	std::condition_variable block_ready;
	mutable std::mutex queue_mtx;
	std::deque<data_map_type> block_queue;

	std::thread persist_thread;
	std::atomic<bool> done;
};

#endif //BLOCK_QUEUE_HPP_