
#include <chrono>
#include "client.hpp"

int main()
{
    Client client("localhost", 9192);
    client.start();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

    return 0;
}