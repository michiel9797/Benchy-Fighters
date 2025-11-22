//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 30-07-2025

#include "network_layer.h"
#include <stdio.h>

using json = nlohmann::json;

	
jsonMessage::jsonMessage()
{
	data = json::object();
}//jsonMessage

template<typename Stream> bool jsonMessage::Serialize(Stream & stream)
{
	std::string jsonString;

	if(Stream::IsWriting)                                
  {                                                       
    jsonString = data.dump();                 
  }//if  

  // Serialize the length
  uint16_t length = jsonString.size();
  serialize_int(stream, length, 0, maxMessageBuffer);

  // Drop if the message is too long
  if (length > maxMessageBuffer)
      return false;

  if (Stream::IsWriting)
  {
    for (int i = 0; i < length; ++i)
        serialize_bits(stream, jsonString[i], 8);

  }else{ //reading  
    std::string readString;
    readString.resize(length);

    for (int i = 0; i < length; ++i)
        serialize_bits(stream, readString[i], 8);

    data = json::parse(readString);                             
	}//if         
	return true;
}//serialize



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