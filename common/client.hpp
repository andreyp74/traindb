#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <string>
#include <iostream>
#include <chrono>
#include <iterator>

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"

#include "net.hpp"

namespace net
{
    using namespace Poco;
    using namespace Poco::Net;

    class Client
    {
    public:
        Client(const std::string& host, Poco::UInt16 port):
            socket_addr(host, port)
        {
        }

        bool connect()
        {
            try {
                socket.connect(socket_addr); 
                //std::cout << "Connected to " << socket_addr.host << ":" << socket_addr.port << std::endl;
                return true;
            }
            catch(Poco::Exception err) {
                std::cerr << "Connection failed: " << err.what() << std::endl;
            }
            return false;
        }

		StreamSocket& get_socket()
		{
			return socket;
		}

        void send(const std::string& request)
        {
            net::send(this->socket, request);
        }

        std::string receive()
        {
            return net::receive(this->socket);
        }

    private:
        Client(Client&) = delete;
        Client& operator=(Client&) = delete;

    private:
        // IP endpoint/socket address (consists of host addr and port #)
        SocketAddress socket_addr;

        // Interface to a TCP stream socket
        StreamSocket socket;
    };

}
#endif //CLIENT_HPP_