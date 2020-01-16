
#include <chrono>

#include "Poco/Util/ServerApplication.h"

#include "proto.hpp"
#include "write_server.hpp"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace proto;

Packet WriteServer::receive()
{
	std::string msg = net::receive(this->socket());
    Application::instance().logger().information("Received request: " + msg);
    return deserialize(msg);
}

void WriteServer::send(const Packet& packet)
{
    auto msg = serialize(packet);
	net::send(this->socket(), msg);
    Application::instance().logger().information("Sent: " + msg);
}

void WriteServer::on_recv(const std::string& msg)
{
	Application::instance().logger().information("Received: " + msg);

	Packet packet = deserialize(msg);

	if (packet.packet_type == PacketType::Ack)
		storage->commit(packet.entry.key, packet.entry.version);
}

void WriteServer::commit_version(const Packet& packet)
{
	storage->commit(packet.entry.key, packet.entry.version);
	Packet ack(PacketType::Ack, packet.entry);
	send(ack);
}

void WriteServer::run()
{
    Application& app = Application::instance();
    app.logger().information("Connection from " + this->socket().peerAddress().toString());

    while(true)
    {
        try
        {
            app.logger().information("Waiting for request...");
            Packet packet = receive();
            if (packet.packet_type == PacketType::Set)
            {
                if (packet.entry.version == -1)
                    packet.entry.version = std::chrono::system_clock::now().time_since_epoch().count();

                storage->put_data(packet.entry.key, packet.entry.value, packet.entry.version);

                if (succ_client)
                {
                    succ_client->enqueue({ packet.entry.key, packet.entry.value, packet.entry.version });
                }
                else
                {
					commit_version(packet);
                }
            }
            else if (packet.packet_type == PacketType::Ack)
            {
                storage->commit(packet.entry.key, packet.entry.version);
            }
            else
            {
                throw std::runtime_error("Unsupported request");
            }
        }
        catch (Poco::Exception& exc)
        {
            //TODO send deny
            app.logger().error(exc.displayText());
        }
    }

    app.logger().information("Connection is closing");
}