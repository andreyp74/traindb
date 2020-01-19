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
#include "succ_client.hpp"


using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;


class TraindbApp : public ServerApplication
{
protected:
	struct ProgramOptions
	{
		int read_port;
		int write_port;
		int succ_port;
		std::string succ_host;
	};

	ProgramOptions parse_command_line(const std::vector<std::string>& args)
	{
		ProgramOptions options;
		auto end = args.end();
		for(auto it = args.begin(); it != end; ++it)
		{
			if (*it == "--rd_port" && ++it != end)
			{
				options.read_port = std::stoi(*it);
			}
			else if (*it == "--wr_port" && ++it != end)
			{
				options.write_port = std::stoi(*it);
			}
			else if (*it == "--succ_port" && ++it != end)
			{
				options.succ_port = std::stoi(*it);
			}
			else if (*it == "--succ_host" && ++it != end)
			{
				options.succ_host = *it;
			}
		}
		return options;
	}

	int main(const std::vector<std::string>& args)
	{
		ProgramOptions options = parse_command_line(args);

		std::shared_ptr<Storage> storage = std::make_shared<Storage>();

		ServerSocket read_socket((Poco::UInt16)options.read_port);
		TCPServer read_server(new ReadServerFactory(storage), read_socket);
		read_server.start();
		std::cout << "ReadServer started on port: " << options.read_port << std::endl;

		std::shared_ptr<SuccClient> succ_client;
		if (options.succ_port && !options.succ_host.empty())
		{
			succ_client = std::make_shared<SuccClient>(options.succ_host, (Poco::UInt16)options.succ_port);
			succ_client->start();
			std::cout << "Connected to successor: " << options.succ_host << ":" << options.succ_port << std::endl;
		}

		ServerSocket write_socket((Poco::UInt16)options.write_port);
		TCPServer write_server(new WriteServerFactory(storage, succ_client), write_socket);
		write_server.start();
		std::cout << "WriteServer started on port: " << options.write_port << std::endl;


		// wait for CTRL-C or kill
		waitForTerminationRequest();

		read_server.stop();
		write_server.stop();

		return Application::EXIT_OK;
	}
};


int main(int argc, char** argv)
{
	TraindbApp app;
	return app.run(argc, argv);
}
