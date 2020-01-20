#ifndef WRITE_CLIENT_HPP_
#define WRITE_CLIENT_HPP_

#include <thread>
#include <string>
#include <atomic>
#include <chrono>

#include "client.hpp"
#include "proto.hpp"

class WriteClient
{
public:
	WriteClient(int port, const std::string& host) :
		client(host, (Poco::UInt16)port),
		done(false)
	{
		std::srand((unsigned int)std::time(nullptr));
	}

	~WriteClient()
	{
		stop();
	}

	void start()
	{
		client.connect();
		client_thread = std::thread(&WriteClient::run, this);

		if (client_thread.joinable())
			client_thread.join();
	}

	void stop()
	{
		done = true;
	}

private:
	void run()
	{
		using namespace std::chrono_literals;

		while (!done)
		{
			try
			{
				std::string key = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
				std::string value = std::to_string(std::rand());

				Entry entry{ key, value, -1 };
				proto::Packet packet(proto::PacketType::Set, entry);
				client.send(packet);

				std::cout << "Sent: packet_type: " << packet.packet_type << ", key: " << entry.key << ", value: " << entry.value << std::endl;

				proto::Packet resp = client.receive();
				std::cout << "Response: packet_type: " << std::to_string(resp.packet_type)
					<< ", key: " << resp.entry.key
					<< ", value: " << resp.entry.value
					<< ", version: " << resp.entry.version << std::endl;

				std::this_thread::sleep_for(1000ms);
			}
			catch (std::exception& err)
			{
				std::cout << "Error occurred: " << err.what() << std::endl;
			}
		}
	}

private:
	net::Client client;
	std::atomic<bool> done;
	std::thread client_thread;
};

#endif //WRITE_CLIENT_HPP_
