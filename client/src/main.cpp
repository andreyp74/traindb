
#include <chrono>
#include <iostream>
#include <sstream>

#include "read_client.hpp"
#include "write_client.hpp"

using namespace proto;

enum Mode
{
	READ_MODE = 0,
	WRITE_MODE
};

struct ProgramOptions 
{
	int port;
	std::string host;
	Mode mode;

};

ProgramOptions parse_command_line(int argc, char** argv)
{
	ProgramOptions options;
	for (int i = 0; i < argc;)
	{
		if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
		{
			options.port = std::stoi(argv[++i]);
		}
		else if (strcmp(argv[i], "--host") == 0 && i + 1 < argc)
		{
			options.host = argv[++i];
		}
		else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc)
		{
			options.mode = strcmp(argv[++i], "read") == 0 ? READ_MODE : WRITE_MODE;
		}
		else
		{
			++i;
		}
	}
	return options;
}


int main(int argc, char** argv)
{
	using namespace std::chrono_literals;

	try
	{
		ProgramOptions options = parse_command_line(argc, argv);
		if (options.mode == Mode::READ_MODE)
		{
			ReadClient read_client(options.port, options.host);
			read_client.start();
		}
		else if (options.mode == Mode::WRITE_MODE)
		{
			WriteClient write_client(options.port, options.host);
			write_client.start();
		}
	}
	catch (std::exception& err)
	{
		std::cerr << "Error occurred: " << err.what() << std::endl;
	}

    return 0;
}