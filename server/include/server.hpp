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
#include "Poco/JSON/Parser.h"

#include "storage.hpp"
#include "net.hpp"

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

	void run() override
	{
		Application& app = Application::instance();
		app.logger().information("Connection from " + this->socket().peerAddress().toString());

		while(true)
		{
			try
			{	
				app.logger().information("Waiting for request...");
				std::string json = receive();
				app.logger().information("Received request: " + json);

				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result = parser.parse(json);
				Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
				std::string request = object->getValue<std::string>("request");

				std::string response_json;
				if (request == "get")
				{
					std::string key = object->getValue<std::string>("key");

					std::string value = storage->get_data(key);
					response_json = "{ \"result\" : \"ok\", \"value\" : \"" + value + "\" }";
				}
				else if (request == "set")
				{
					std::string key = object->getValue<std::string>("key");
					std::string value = object->getValue<std::string>("value");

					storage->put_data(key, std::move(value));
					response_json = "{ \"result\" : \"ok\" }";
				}
			
				send(response_json);

				app.logger().information("Sent " + response_json);
			}
			catch (Poco::Exception& exc)
			{
				std::string msg = "{ \"result\" : \"error\", \"message\" : \"" + exc.displayText() + "\"}";
				send(msg);

				app.logger().log(exc);
			}
		}

		app.logger().information("Connection is closing");
	}

private:
	std::string receive()
	{
		return net::receive(this->socket());
	}

	void send(const std::string& msg)
	{
		net::send(this->socket(), msg);
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