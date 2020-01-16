#include "Poco/Util/ServerApplication.h"
#include "Poco/JSON/Parser.h"

#include "read_server.hpp"
#include "proto.hpp"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;
using namespace proto;

Packet ReadServer::receive()
{
	std::string msg = net::receive(this->socket());
    Application::instance().logger().information("Received request: " + msg);
    return deserialize(msg);
}

void ReadServer::send(const Packet& packet)
{
    auto msg = serialize(packet);
	net::send(this->socket(), msg);
    Application::instance().logger().information("Sent: " + msg);
}

void ReadServer::run()
{
    Application& app = Application::instance();
    app.logger().information("Connection from " + this->socket().peerAddress().toString());

    while(true)
    {
        try
        {
            app.logger().information("Waiting for request...");
            Packet packet = receive();
            if (packet.packet_type == PacketType::Get)
            {
				value_type value;
				if (storage->get_data(packet.entry.key, value))
				{
					//TODO: return from storage and send result with version number ?
					Packet response(PacketType::Result, Entry{ packet.entry.key, value, -1 });
					send(response);
				}
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