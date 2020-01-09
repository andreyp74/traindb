#ifndef READ_SERVER_HPP_
#define READ_SERVER_HPP_

#include <string>

#include "Poco/Net/TCPServerConnectionFactory.h"

#include "server_connection.hpp"

class ReadServer: public ServerConnection
{
public:
	ReadServer(const Poco::Net::StreamSocket& s, std::shared_ptr<Storage> storage) :
		ServerConnection(s, storage)
	{}

	void run() override;
};

class ReadServerFactory: public Poco::Net::TCPServerConnectionFactory
{
public:
	ReadServerFactory(std::shared_ptr<Storage> storage) :
		storage(storage)
	{}

	 Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override
	{
		return new ReadServer(socket, storage);
	}

private:
	std::shared_ptr<Storage> storage;
};


#endif //READ_SERVER_HPP_