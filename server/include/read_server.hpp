#ifndef READ_SERVER_HPP_
#define READ_SERVER_HPP_

#include <string>

#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/StreamSocket.h"

#include "storage.hpp"
#include "net.hpp"
#include "succ_client.hpp"

class ReadServer: public Poco::Net::TCPServerConnection
{
public:
	ReadServer(const Poco::Net::StreamSocket& s, std::shared_ptr<Storage> storage) :
		Poco::Net::TCPServerConnection(s), 
		storage(storage)
	{}

	virtual ~ReadServer() {}

	void run() override;

private:

	proto::Packet receive();
	void send(const proto::Packet& packet);

private:
	std::shared_ptr<Storage> storage;
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