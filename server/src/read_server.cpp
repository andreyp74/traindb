#include "read_server.hpp"

#include "Poco/Util/ServerApplication.h"
#include "Poco/JSON/Parser.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

void ReadServer::run()
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
            if (request == "get")
            {
                std::string key = object->getValue<std::string>("key");

                std::vector<std::string> resp = storage->get_data(key);
                std::string values;
                for (auto& value : resp)
                {
                    if (!values.empty())
                        values += ",";
                    values += "\"";
                    values += value;
                    values += "\"";
                }
                response_json = "{ \"result\" : \"ok\", \"values\" : [" + values + "] }";
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