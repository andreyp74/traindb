#ifndef SUCC_CLIENT_HPP_
#define SUCC_CLIENT_HPP_

#include <atomic>
#include <mutex>
#include <thread>
#include <deque>
#include <condition_variable>
#include <functional>

#include "Poco/Types.h"

#include "storage_types.hpp"
#include "storage.hpp"
#include "client.hpp"
#include "entry.hpp"

class SuccClient
{
public:

	SuccClient(const std::string& host, Poco::UInt16 port);

	void start();
	void stop();
	void enqueue(Entry&& item);

	void register_callback(std::function<void(const proto::Packet&)>& call_back);

private:
    void run();

private:
	net::Client client;

	std::thread succ_thread;

	std::atomic<bool> done;

	std::mutex queue_mtx;
	std::deque<Entry> queue;
	std::condition_variable queue_ready;

	std::vector<std::function<void(const proto::Packet&)>> call_backs;
};

#endif //SUCC_CLIENT_HPP_