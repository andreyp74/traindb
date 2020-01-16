#ifndef WRITE_SERVER_HPP_
#define WRITE_SERVER_HPP_

#include <string>

#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/StreamSocket.h"

#include "storage.hpp"
#include "net.hpp"
#include "succ_client.hpp"

class WriteServer: public Poco::Net::TCPServerConnection
{
public:
	WriteServer(const Poco::Net::StreamSocket& s,
		std::shared_ptr<Storage> storage,
		std::shared_ptr<SuccClient> succ_client);

	virtual ~WriteServer() {}

	void run() override;

private:

	void commit_version(const proto::Packet& packet);

	proto::Packet receive();
	void send(const proto::Packet& packet);

	void on_succ_receive(const proto::Packet& packet);

private:
	std::shared_ptr<Storage> storage;
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