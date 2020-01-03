#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <atomic>
#include <memory>
#include <assert.h>
#include <iostream>
#include <iterator> 
#include <algorithm>

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Exception.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"

#include "storage.hpp"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

class ServerConnection: public TCPServerConnection
	/// This class handles all client connections.
{
public:
	ServerConnection(const StreamSocket& s, 
		             std::shared_ptr<Storage> storage) : 
		TCPServerConnection(s),
		storage(storage)
	{}

	void run()
	{
		Application& app = Application::instance();
		app.logger().information("Connection from " + this->socket().peerAddress().toString());
		while(true)
		{
			try
			{	
				app.logger().information("Waiting for request...");
				std::string key = recv_request();

				app.logger().information("Received request: " + key);
				auto msg = storage->get_data(key);
			
				send_response(msg);

				app.logger().information("Sent " + std::to_string(msg.size()) + " items");
				std::ostream_iterator<std::string> out_it(std::cout, "\n");
				std::copy(msg.begin(), msg.end(), out_it);
			}
			catch (Poco::Exception& exc)
			{
				app.logger().log(exc);
			}
		}

		app.logger().information("Connection is closing");
	}

private:
	std::string recv_request()
	{
		unsigned char buffer[sizeof(size_t)];
		this->socket().receiveBytes(buffer, sizeof(buffer));
		size_t size = *(size_t*)&buffer;

        std::string buff;
        buff.resize(size);
        int received_bytes = 0;
		while (received_bytes < size)
		{
			received_bytes += this->socket().receiveBytes(buff.data() + received_bytes, size - received_bytes);
		}
		return buff;
	}

	void send_response(const std::vector<std::string>& msg)
	{
		size_t msg_size = msg.size();
		this->socket().sendBytes(&msg_size, sizeof(size_t));

		for(auto v: msg)
		{
			size_t value_size = v.size();
			this->socket().sendBytes(&value_size, sizeof(size_t));
			this->socket().sendBytes(v.data(), value_size);
		}
	}

private:
	std::shared_ptr<Storage> storage;
};

class ServerConnectionFactory: public TCPServerConnectionFactory
{
public:
	ServerConnectionFactory(std::shared_ptr<Storage> storage) : 
		storage(storage)
	{}

	TCPServerConnection* createConnection(const StreamSocket& socket) override
	{
		return new ServerConnection(socket, storage);
	}

private:
	std::shared_ptr<Storage> storage;
};

class Server: public ServerApplication
{
protected:

	int main(const std::vector<std::string>&)
	{
		std::shared_ptr<Storage> storage = std::make_shared<Storage>();
	
		// get parameters from configuration file
		unsigned short port = (unsigned short)config().getInt("Server.port", 9192);

		// set-up a server socket
		ServerSocket svs(port);
		//svs.setBlocking(false);

		// set-up a TCPServer instance
		TCPServer srv(new ServerConnectionFactory(storage), svs);
		// start the TCPServer
		srv.start();

		std::cout << "Server started on port: " << port << std::endl;

		// wait for CTRL-C or kill
		waitForTerminationRequest();

		// Stop the TCPServer
		srv.stop();
		
		return Application::EXIT_OK;
	}
};

#endif //SERVER_HPP_