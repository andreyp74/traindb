#include "Poco/JSON/Parser.h"

#include "succ_client.hpp"

using namespace Poco;

SuccClient::SuccClient(const std::string& host, Poco::UInt16 port, std::shared_ptr<Storage> storage) :
	client(host, port),
	storage(storage),
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

void SuccClient::enqueue(SuccClient::QueueItem&& item)
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
			QueueItem item = std::move(queue.front());
			queue.pop_front();

			lock.unlock();

			//--> TODO: move to some common protocol part
			std::string request = "{ \"pack\" : \"set\", \"key\" : \"" + item.key + "\", \"value\" : \"" + item.value + "\", \"version\":\"" + std::to_string(item.version) + "\" }";

			client.send(request);
			std::string response = client.receive();

			JSON::Parser parser;
			Dynamic::Var result = parser.parse(response);
			JSON::Object::Ptr object = result.extract<JSON::Object::Ptr>();
			std::string pack = object->getValue<std::string>("pack");

			std::string response_json;
			if (pack == "ack")
			{
				std::string key = object->getValue<std::string>("key");
				//std::string value = object->getValue<std::string>("value");
				ver_type version = object->getValue<ver_type>("version");

				//TODO: I am not glad with that solution
				if (!storage.expired())
					storage.lock()->commit(key, version);
			}
			//<--

			lock.lock();
		}
	}
}