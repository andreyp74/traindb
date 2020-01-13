
#include <chrono>

#include "write_server.hpp"

#include "Poco/Util/ServerApplication.h"
#include "Poco/JSON/Parser.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

void WriteServer::run()
{
    Application& app = Application::instance();
    app.logger().information("Connection from " + this->socket().peerAddress().toString());

    while(true)
    {
        try
        {
            app.logger().information("Waiting for request...");
            std::string json = receive();
            app.logger().information("Received request: " + json);

            JSON::Parser parser;
            Dynamic::Var result = parser.parse(json);
            JSON::Object::Ptr object = result.extract<JSON::Object::Ptr>();
            std::string request = object->getValue<std::string>("pack");

            std::string response_json;
            if (request == "set")
            {
                std::string key = object->getValue<std::string>("key");
                std::string value = object->getValue<std::string>("value");

				ver_type version;
				if (!object->isNull("version"))
					version = object->getValue<ver_type>("version");
				else 
					version = std::chrono::system_clock::now().time_since_epoch().count();

				storage->put_data(key, value, version);

				if (succ_client)
				{
					succ_client->enqueue(key, value, version);
				}
				else
				{
					storage->commit(key, version);
					response_json = "{ \"pack\" : \"ack\", \"key\": \"" + key + "\", \"value\": \"" + value + "\", \"version\": \"" + std::to_string(version) + "\"}";
				}
            }
            else
            {
                throw std::runtime_error("Unsupported request");
            }

            send(response_json);

            app.logger().information("Sent " + response_json);
        }
        catch (Poco::Exception& exc)
        {
            std::string msg = "{ \"result\" : \"error\", \"message\" : \"" + exc.displayText() + "\"}";
            send(msg);

            app.logger().log(exc);
        }
    }

    app.logger().information("Connection is closing");
}