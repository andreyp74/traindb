#include "Poco/JSON/Parser.h"

#include "proto.hpp"
#include "succ_client.hpp"

using namespace Poco;
using namespace proto;

SuccClient::SuccClient(const std::string& host, Poco::UInt16 port) :
	client(host, port),
	send_done(false),
	recv_done(false)
{
}

void SuccClient::start()
{
	client.connect();

	send_thread = std::thread(&SuccClient::send_run, this);
	recv_thread = std::thread(&SuccClient::recv_run, this);
}

void SuccClient::stop()
{
	queue_ready.notify_one();

	send_done = true;
	if (send_thread.joinable())
		send_thread.join();

	recv_done = true;
	if (recv_thread.joinable())
		recv_thread.join();
}

void SuccClient::enqueue(Entry&& item)
{
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		queue.push_back(std::move(item));
	}
	queue_ready.notify_one();
}

void SuccClient::send_run()
{
	while (!send_done)
	{
		std::unique_lock<std::mutex> lock(queue_mtx);
		queue_ready.wait(lock, [this]() { return send_done || !queue.empty(); });

		while (!queue.empty())
		{
			Entry entry = std::move(queue.front());
			queue.pop_front();

			lock.unlock();

			Packet packet(PacketType::Ack, entry);
			Poco::Timespan timeout(10, 0);
			if (client.get_socket().poll(timeout, Poco::Net::Socket::SELECT_WRITE))
				client.send(packet);

			lock.lock();
		}
	}
}

void SuccClient::register_callback(std::function<void(const proto::Packet&)>& call_back)
{
	call_backs.push_back(call_back);
}

void SuccClient::recv_run()
{
	while (!recv_done)
	{
		Poco::Timespan timeout(10, 0);
		if (client.get_socket().poll(timeout, Poco::Net::Socket::SELECT_READ))
		{
			Packet packet = client.receive();
			for (auto& call_back : call_backs)
			{
				call_back(packet);
			}
		}
	}
}