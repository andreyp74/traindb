#include "Poco/JSON/Parser.h"

#include "proto.hpp"
#include "succ_client.hpp"

using namespace Poco;
using namespace proto;

SuccClient::SuccClient(const std::string& host, Poco::UInt16 port) :
	client(host, port),
	done(false)
{
}

void SuccClient::start()
{
	client.connect();
	succ_thread = std::thread(&SuccClient::run, this);
}

void SuccClient::stop()
{
	queue_ready.notify_one();

	done = true;
	if (succ_thread.joinable())
		succ_thread.join();
}

void SuccClient::enqueue(Entry&& item)
{
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		queue.push_back(std::move(item));
	}
	queue_ready.notify_one();
}

void SuccClient::run()
{
	while (!done)
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		queue_ready.wait(lock, [this]() { return done || !queue.empty(); });

		while (!queue.empty())
		{
			Entry entry = std::move(queue.front());
			queue.pop_front();

			lock.unlock();

			Poco::Timespan timeout;
			if (client.get_socket().poll(timeout, Poco::Net::Socket::SELECT_READ))
			{
				Packet packet = client.receive();
				for (auto& call_back : call_backs)
				{
					call_back(packet);
				}
			}
			else
			{
				Packet packet(PacketType::Ack, entry);
				client.send(packet);
			}
			
			lock.lock();
		}
	}
}

void SuccClient::register_callback(std::function<void(const proto::Packet&)>& call_back)
{
	call_backs.push_back(call_back);
}
