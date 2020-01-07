
#include <chrono>
#include <iostream>
#include <sstream>

#include "client.hpp"

int main()
{
    net::Client client("localhost", 9192);
	if (!client.connect())
		std::cerr << "Couldn't connect to server" << std::endl;

	while (true)
	{
		try
		{
			std::string text;
			std::getline(std::cin, text);

			std::istringstream iss(text);
			std::vector<std::string> request((std::istream_iterator<std::string>(iss)),
				std::istream_iterator<std::string>());

			size_t size = request.size();
			std::string request_json;
			if (request[0] == "get")
			{
				if (size < 2)
					throw std::runtime_error("Incorrect get request");

				request_json = "{ \"request\" : \"get\", \"key\" : \"" + request[1] + "\" }";
			}
			else if (request[0] == "set")
			{
				if (size < 3)
					throw std::runtime_error("Incorrect set request");

				request_json = "{ \"request\" : \"set\", \"key\" : \"" + request[1] + "\", \"value\" : \"" + request[2] + "\" }";
			}
			
			net::send(client.get_socket(), request_json);

			std::string response_json = client.receive();
			std::cout << "Received " << response_json << std::endl;
		}
		catch (Poco::Exception err)
		{
			std::cerr << "Error occurred: " << err.what() << std::endl;
		}
	}

    return 0;
}