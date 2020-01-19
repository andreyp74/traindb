#ifndef PROTO_HPP_
#define PROTO_HPP_

#include "entry.hpp"

#include "Poco/JSON/Parser.h"

namespace proto
{
    enum PacketType
    {
        Unknown = 0,
        Set     = 1,
        Get     = 2,
        Ack     = 3,
        Deny    = 4,
        Result  = 5
    };



    struct Packet
    {
        Packet() : Packet(PacketType::Unknown, Entry()) {}

        Packet(PacketType packet_type, const Entry& entry) :
            packet_type(packet_type),
            entry(entry)
        {
        }

        Packet(PacketType packet_type, Entry&& entry) :
            packet_type(packet_type),
            entry(std::move(entry))
        {
        }

        PacketType packet_type;
        Entry entry;
    };

    inline std::string serialize(const Packet& packet)
    {
        std::string json;
        if (packet.packet_type ==  PacketType::Set)
        {
            json = "{ \"pack\" : \"set\", \"key\": \"" + packet.entry.key + 
                "\", \"value\": \"" + packet.entry.value + 
                "\", \"version\": \"" + std::to_string(packet.entry.version) + "\"}";
        }
        else if (packet.packet_type == PacketType::Get)
        {
            json = "{ \"pack\" : \"get\", \"key\": \"" + packet.entry.key + "\"}";
        }
        else if (packet.packet_type == PacketType::Ack)
        {
            json = "{ \"pack\" : \"ack\", \"key\": \"" + packet.entry.key + 
                "\", \"value\": \"" + packet.entry.value + 
                "\", \"version\": \"" + std::to_string(packet.entry.version) + "\"}";
        }
        else if (packet.packet_type == PacketType::Deny)
        {
            json = "{ \"pack\" : \"deny\", \"key\": \"" + packet.entry.key + 
                "\", \"value\": \"" + packet.entry.value + 
                "\", \"version\": \"" + std::to_string(packet.entry.version) + "\"}";
        }
        else if (packet.packet_type == PacketType::Result)
        {
            json = "{ \"pack\" : \"result\", \"key\": \"" + packet.entry.key + 
                "\", \"value\": \"" + packet.entry.value + "\"}";
        }
        else
        {
            //TODO Exception?
        }
		return json;
    }

    inline Packet deserialize(const std::string& msg)
    {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(msg);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        std::string pack = object->getValue<std::string>("pack");

        Packet packet;
        if (pack == "set")
            packet.packet_type = PacketType::Set;
        else if (pack == "get")
            packet.packet_type = PacketType::Get;
        else if (pack == "ack")
            packet.packet_type = PacketType::Ack;
        else if (pack == "deny")   
            packet.packet_type = PacketType::Deny;
        else if (pack == "result")   
            packet.packet_type = PacketType::Result;
        else
            //TODO Exception?
            packet.packet_type = PacketType::Unknown;

        packet.entry.key = std::move(object->getValue<key_type>("key"));

        if (!object->isNull("value"))
            packet.entry.value = std::move(object->getValue<value_type>("value"));

        if (!object->isNull("version"))
            packet.entry.version = object->getValue<ver_type>("version");
        else
            packet.entry.version = -1;

        return packet;
    }
}


#endif //PROTO_HPP_