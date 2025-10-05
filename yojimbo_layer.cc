//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 30-07-2025

#include "network_layer.h"
#include <stdio.h>

using json = nlohmann::json;

struct jsonMessage : public yojimbo::Message
{	
	json data;
	
	jsonMessage()
	{
		data = json::object();
	}//jsonMessage

	template<typename Stream> bool Serialize(Stream & stream)
	{
		std::string jsonString;

		if(Stream::IsWriting)                                
        {                                                       
          jsonString = data.dump();                 
        }//if  

		// Allocate max buffer size (change if needed)
        const int maxJsonStringLength = 1024;

        // Allocate a buffer to hold the string
        char buffer[maxJsonStringLength];
        memset(buffer, 0, maxJsonStringLength);

        if (Stream::IsWriting)
        {
            size_t length = std::min((int)jsonString.size(), maxJsonStringLength - 1);
            memcpy(buffer, jsonString.c_str(), length);
        }//if

        for (int i = 0; i < maxJsonStringLength; ++i)
        {
            serialize_bits(stream, buffer[i], 8);
		}//for
                                               
        if(Stream::IsReading)                                
        {                                                       
			data = json::parse(buffer);                             
		}//if         
		return true;
	}//serialize

	YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS()

};//jsonMessage

enum messageEnum
{
	JSON_MESSAGE,
	NUM_MESSAGE_TYPES
};//messageEnum

YOJIMBO_MESSAGE_FACTORY_START(YojimboMessageFactory, NUM_MESSAGE_TYPES);
YOJIMBO_DECLARE_MESSAGE_TYPE( JSON_MESSAGE, jsonMessage);
YOJIMBO_MESSAGE_FACTORY_FINISH();

yojimbo::MessageFactory * yojimboAdapter::CreateMessageFactory(yojimbo::Allocator & allocator)
{
	return YOJIMBO_NEW(allocator, YojimboMessageFactory, allocator);
}//CreateMessageFactory

networkLayer::~networkLayer()
{
	//empty for now
}

bool yojimboLayer::startConnection()
{
	return false;
}//startConnection

void yojimboLayer::sendMessage(std::vector<std::pair<char, float>> input)
{
	return;
}//sendMessage

std::vector<std::pair<char, float>> yojimboLayer::receiveMessage()
{
	std::vector<std::pair<char, float>> temp;
	return temp;
}//receiveMessage

void yojimboLayer::endConnection()
{
	return;
}//endConnections

void yojimboServer::startMatch()
{
	return;
}//startMatch

void yojimboServer::endMatch()
{
	return;
}//endMatch