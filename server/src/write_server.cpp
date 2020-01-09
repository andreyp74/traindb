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
            std::string request = object->getValue<std::string>("request");

            std::string response_json;
            if (request == "set")
            {
                std::string key = object->getValue<std::string>("key");
                std::string value = object->getValue<std::string>("value");

                storage->put_data(key, std::move(value));
                response_json = "{ \"result\" : \"ok\" }";
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