#ifndef WRITE_SERVER_HPP_
#define WRITE_SERVER_HPP_

#include <string>

#include "Poco/Net/TCPServerConnectionFactory.h"

#include "server_connection.hpp"

class WriteServer: public ServerConnection
{
public:
	WriteServer(const Poco::Net::StreamSocket& s, std::shared_ptr<Storage> storage) :
		ServerConnection(s, storage)
	{}

	void run() override;
};

class WriteServerFactory: public Poco::Net::TCPServerConnectionFactory
{
public:
	WriteServerFactory(std::shared_ptr<Storage> storage) :
		storage(storage)
	{}

	Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override
	{
		return new WriteServer(socket, storage);
	}

private:
	std::shared_ptr<Storage> storage;
};


#endif //WRITE_SERVER_HPP_