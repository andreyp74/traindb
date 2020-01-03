#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <iterator>

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"

using namespace Poco;
using namespace Poco::Net;

class Client
{
public:
    Client(const std::string& host, Poco::UInt16 port):
        socket_addr(host, port), 
        host_(host), 
        port_(port),
        done(false)
    {
    }

    ~Client()
    {
        stop();
    }

    void start()
    {
        while (! connect())
			std::this_thread::sleep_for(std::chrono::seconds(5));
        client_thread = std::thread(&Client::run, this);
    }

    void stop()
    {
        done = true;
        if (client_thread.joinable())
            client_thread.join();
    }

    void run()
    {
        while(!done)
        {
			try
			{
				int64_t end_time = std::chrono::system_clock::now().time_since_epoch().count();
				int64_t start_time = end_time - 1 * 1000000;
				std::cout << "Requesting from " << start_time << " to " << end_time << std::endl;
				send_time_period(start_time, end_time);

				std::vector<int> dt = recv_data();
				std::cout << "Received " << dt.size() << " items" << std::endl;
				std::ostream_iterator<int> out_it(std::cout, "\n");
				std::copy(dt.begin(), dt.end(), out_it);
			}
			catch (Poco::Exception err)
			{
				std::cerr << "Error occurred: " << err.what() << std::endl;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    bool connect()
    {
        try {
            socket.connect(socket_addr); 
            std::cout << "Connected to " << host_ << ":" << port_ << std::endl;
            return true;
        }
        catch(Poco::Exception err) {
            std::cerr << "Connection failed: " << err.what() << std::endl;
        }
        return false;
    }

private:
    Client(Client&) = delete;
    Client& operator=(Client&) = delete;

	void send_time_period(int64_t start_time, int64_t end_time)
	{
		socket.sendBytes(&start_time, (int)sizeof(start_time));
		socket.sendBytes(&end_time, (int)sizeof(end_time));
	}

	std::vector<int> recv_data()
	{
		unsigned char buffer[sizeof(size_t)];
		int received_bytes = socket.receiveBytes(buffer, sizeof(buffer));
		size_t size = *(size_t*)&buffer;

        std::vector<int> dt;
        dt.resize(size/sizeof(int));
        received_bytes = 0;
		while (received_bytes < size)
		{
			received_bytes += socket.receiveBytes(dt.data() + received_bytes, size - received_bytes);
		}

		return dt;
	}

private:
    // IP endpoint/socket address (consists of host addr and port #)
	SocketAddress socket_addr;

	// Interface to a TCP stream socket
	StreamSocket socket;

    std::thread client_thread;

    std::string host_;
    int port_;

    std::atomic<bool> done;
};

#endif //CLIENT_HPP_