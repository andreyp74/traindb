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
#include "entry.hpp"

class SuccClient
{
public:

	SuccClient(const std::string& host, Poco::UInt16 port, std::shared_ptr<Storage> storage);

	void start();
	void stop();
	void enqueue(Entry&& item);

private:
    void run();

private:
	net::Client client;

	std::thread client_thread;

	std::atomic<bool> done;
	std::mutex queue_mtx;
	std::deque<Entry> queue;
	std::condition_variable queue_ready;

	std::weak_ptr<Storage> storage;

};

#endif //SUCC_CLIENT_HPP_