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
#include "client.hpp"

class SuccClient
{
public:

	using queue_item_type = std::pair<std::string, std::string>;

	SuccClient(const std::string& host, Poco::UInt16 port);

	void start();
	void stop();
	std::future<queue_item_type> enqueue(const std::string& key, const std::string& value, ver_type version);

private:
    void run();

private:
	net::Client client;

	std::thread client_thread;

	std::atomic<bool> done;
	std::mutex queue_mtx;
	std::deque<queue_item_type> queue;
	std::condition_variable queue_ready;

};

#endif //SUCC_CLIENT_HPP_