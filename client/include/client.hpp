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
        port_(port)
    {
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

	void send(const std::string& request)
	{
		size_t size = request.size();
		socket.sendBytes(&size, (int)sizeof(size_t));
		socket.sendBytes(request.data(), (int)size);
	}

	std::string receive()
	{
		unsigned char buffer[sizeof(size_t)];
		socket.receiveBytes(buffer, sizeof(buffer));
		size_t size = *(size_t*)&buffer;

		std::string msg;
		msg.resize(size);
		int received_bytes = 0;
		while (received_bytes < size)
		{
			received_bytes += socket.receiveBytes(msg.data() + received_bytes, size - received_bytes);
		}
		return msg;
	}

private:
    Client(Client&) = delete;
    Client& operator=(Client&) = delete;

private:
    // IP endpoint/socket address (consists of host addr and port #)
	SocketAddress socket_addr;

	// Interface to a TCP stream socket
	StreamSocket socket;

    std::thread client_thread;

    std::string host_;
    int port_;
};

#endif //CLIENT_HPP_