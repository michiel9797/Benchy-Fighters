//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 30-07-2025

#include "network_layer.h"
#include <stdio.h>

using json = nlohmann::json;

	
jsonMessage::jsonMessage()
  : data(json::object())
{
  //no further initialization needed
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

clientLayer::clientLayer(int thisDevice)
  : client(thisDevice)
{
  //no further initialization needed
}//clientLayer

yojimboClient::yojimboClient(int thisDevice)
  : clientLayer(thisDevice)
{
  InitializeYojimbo();
}//yojimboClient

bool yojimboClient::startConnection()
{
	return false;
}//startConnection

void yojimboClient::sendMessage(json message)
{
	return;
}//sendMessage

bool yojimboClient::receiveMessage(json &message)
{
	return false;
}//receiveMessage

void yojimboClient::endConnection()
{
	return;
}//endConnections

void yojimboServer::startMatch()
{
	return;
}//startMatch