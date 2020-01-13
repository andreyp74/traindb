#ifndef WRITE_SERVER_HPP_
#define WRITE_SERVER_HPP_

#include <string>

#include "Poco/Net/TCPServerConnectionFactory.h"

#include "server_connection.hpp"
#include "succ_client.hpp"

class WriteServer: public ServerConnection
{
public:
	WriteServer(const Poco::Net::StreamSocket& s, 
				std::shared_ptr<Storage> storage, 
				std::shared_ptr<SuccClient> succ_client) :
		ServerConnection(s, storage), 
		succ_client(succ_client)
	{}

	void run() override;

private:
	std::shared_ptr<SuccClient> succ_client;
};

class WriteServerFactory: public Poco::Net::TCPServerConnectionFactory
{
public:
	WriteServerFactory(std::shared_ptr<Storage> storage, std::shared_ptr<SuccClient> succ_client) :
		storage(storage),
		succ_client(succ_client)
	{}

	Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) override
	{
		return new WriteServer(socket, storage, succ_client);
	}

private:
	std::shared_ptr<Storage> storage;
	std::shared_ptr<SuccClient> succ_client;
};


#endif //WRITE_SERVER_HPP_