#ifndef SUCC_CLIENT_HPP_
#define SUCC_CLIENT_HPP_

#include <atomic>
#include <mutex>
#include <thread>
#include <deque>
#include <condition_variable>
#include <future>

#include "Poco/Types.h"

#include "storage_types.hpp"
#include "storage.hpp"
#include "client.hpp"

class SuccClient
{
public:

	struct QueueItem
	{
		std::string key;
		std::string value;
		ver_type version;
	};

	SuccClient(const std::string& host, Poco::UInt16 port, std::shared_ptr<Storage> storage);

	void start();
	void stop();
	void enqueue(SuccClient::QueueItem&& item);

private:
    void run();

private:
	net::Client client;

	std::thread client_thread;

	std::atomic<bool> done;
	std::mutex queue_mtx;
	std::deque<QueueItem> queue;
	std::condition_variable queue_ready;

	std::weak_ptr<Storage> storage;

};

#endif //SUCC_CLIENT_HPP_