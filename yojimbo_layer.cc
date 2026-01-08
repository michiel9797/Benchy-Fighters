//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 30-07-2025

#include "network_layer.h"
#include <stdio.h>

const uint64_t ProtocolId = 0x0123456789ABCDEFULL;

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

bool yojimboClient::startOfLoop(double simTime)
{
  return true;
}//startOfLoop

bool yojimboClient::endOfLoop()
{
  return true;
}//endOfLoop

void yojimboClient::endConnection()
{
	return;
}//endConnections

yojimboServer::yojimboServer(char *address)
{
  InitializeYojimbo();

  yojimboAdapter adapter;
	yojimbo::ClientServerConfig config;
	config.networkSimulator = false;

  	uint8_t privateKey[yojimbo::KeyBytes];
  	memset( privateKey, 0, yojimbo::KeyBytes );

	  serverInstance = new yojimbo::Server(
		  yojimbo::GetDefaultAllocator(),
		  privateKey,
		  yojimbo::Address(address),
		  config,
		  adapter,
		  ProtocolId
		);

		serverInstance->Start(2);
}//yojimboServer

yojimboServer::~yojimboServer()
{
  serverInstance->Stop();
  delete serverInstance;
}//~yojimboServer

bool yojimboServer::startMatch()
{
  //if two clients are connected
	if(numClients == 2)
	{	//generate and send the start message
		jsonMessage *message1 = (jsonMessage*)serverInstance->CreateMessage(0, JSON_MESSAGE);
		message1->data["Start"] = "true";
		jsonMessage *message2 = (jsonMessage*)serverInstance->CreateMessage(1, JSON_MESSAGE);
		message2->data["Start"] = "true";
		serverInstance->SendMessage(0, 0, message1);
		serverInstance->SendMessage(1, 0, message2);
				
    return true;
	}//if
  return false;
}//startMatch

void yojimboServer::exchangeMessages()
{
  jsonMessage *message = (jsonMessage*)serverInstance->ReceiveMessage(0, 0);
  jsonMessage *copy;
  while(message)
  {
    copy = (jsonMessage*)serverInstance->CreateMessage(1, JSON_MESSAGE);
    copy->data = message->data;
    serverInstance->ReleaseMessage(0, message);

    serverInstance->SendMessage(1, 0, copy);
    message = (jsonMessage*)serverInstance->ReceiveMessage(0, 0);
  }//while
  message = (jsonMessage*)serverInstance->ReceiveMessage(1, 0);
  while(message)
  {
    copy = (jsonMessage*)serverInstance->CreateMessage(0, JSON_MESSAGE);
    copy->data = message->data;
    serverInstance->ReleaseMessage(1, message);

    serverInstance->SendMessage(0, 0, copy);
    message = (jsonMessage*)serverInstance->ReceiveMessage(1, 0);
  }//while
}//exchangeMessages

bool yojimboServer::startOfLoop(double simTime)
{
	serverInstance->AdvanceTime(simTime);
	serverInstance->ReceivePackets();
  int newNumClients = serverInstance->GetNumConnectedClients();
  if(newNumClients < numClients)
  {
		return false;
  }//if
  return true;
}//startOfLoop

bool yojimboServer::endOfLoop()
{
  serverInstance->SendPackets();
  return true;
}//endOfLoop