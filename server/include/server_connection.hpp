#ifndef SERVER_CONNECTION_HPP_
#define SERVER_CONNECTION_HPP_

#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/StreamSocket.h"

#include "storage.hpp"
#include "net.hpp"

class ServerConnection: public Poco::Net::TCPServerConnection
	/// This class handles all client connections.
{
public:
	ServerConnection(const Poco::Net::StreamSocket& s, std::shared_ptr<Storage> storage) :
		Poco::Net::TCPServerConnection(s),
		storage(storage)
	{}

protected:

    virtual ~ServerConnection() {}

	std::string receive()
	{
		return net::receive(this->socket());
	}

	void send(const std::string& msg)
	{
		net::send(this->socket(), msg);
	}

	std::shared_ptr<Storage> storage;
};

#endif //SERVER_CONNECTION_HPP_