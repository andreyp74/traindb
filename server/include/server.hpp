#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <atomic>
#include <memory>
#include <assert.h>
#include <iostream>
#include <iterator>
#include <algorithm>

#include "Poco/Net/TCPServer.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Exception.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"

#include "storage.hpp"
#include "read_server.hpp"
#include "write_server.hpp"
#include "client.hpp"


using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;


class Server: public ServerApplication
{
protected:

	int main(const std::vector<std::string>&)
	{
		std::shared_ptr<Storage> storage = std::make_shared<Storage>();

		unsigned short read_port = (unsigned short)config().getInt("ReadServer.port", 9192);
		unsigned short write_port = (unsigned short)config().getInt("WriteServer.port", 9193);

		ServerSocket read_socket(read_port);
		TCPServer read_server(new ReadServerFactory(storage), read_socket);
		read_server.start();
		std::cout << "ReadServer started on port: " << read_port << std::endl;

		ServerSocket write_socket(write_port);
		TCPServer write_server(new WriteServerFactory(storage), write_socket);
		write_server.start();
		std::cout << "WriteServer started on port: " << write_port << std::endl;

		unsigned short succ_port = (unsigned short)config().getInt("Succ.port", write_port);
		std::string succ_host = config().getString("Succ.host", "localhost");

		if (succ_port && !succ_host.empty())
		{
			net::Client succ_client(succ_host, succ_port);
			succ_client.connect();
			std::cout << "Connected to successor: " << succ_host << ":" << succ_port << std::endl;
		}

		// wait for CTRL-C or kill
		waitForTerminationRequest();

		read_server.stop();
		write_server.stop();

		return Application::EXIT_OK;
	}
};

#endif //SERVER_HPP_