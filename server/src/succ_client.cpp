#include "succ_client.hpp"


SuccClient::SuccClient(const std::string& host, Poco::UInt16 port) :
	client(host, port),
	done(false)
{
}

void SuccClient::start()
{
	client.connect();
	client_thread = std::thread(&SuccClient::run, this);
}

void SuccClient::stop()
{
	queue_ready.notify_one();

	done = true;
	if (client_thread.joinable())
		client_thread.join();
}

std::future<SuccClient::queue_item_type> SuccClient::enqueue(const std::string& key, const std::string& value, ver_type version)
{
	std::promise<queue_item_type> item({ key, value });
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		queue.push_back(std::move(item));
	}
	queue_ready.notify_one();
	return item.get_future();
}

void SuccClient::run()
{
	while (!done)
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		queue_ready.wait(lock, [this]() { return done || !queue.empty(); });

		while (!queue.empty())
		{
			queue_item_type item = std::move(queue.front());
			queue.pop_front();

			lock.unlock();

			std::string request = "{ \"pack\" : \"set\", \"key\" : \"" + request[1] + "\", \"value\" : \"" + request[2] + "\" }";
			client.send();

			lock.lock();
		}
	}
}