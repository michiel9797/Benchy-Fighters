//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 30-07-2025

#include "network_layer.h"
#include "engine.h"
#include <stdio.h>
#include <thread>

#include <iostream> //!!!!!!!!!!!!!REMOVE

const uint64_t ProtocolId = 0x0123456789ABCDEFULL;

using json = nlohmann::json;

//REMOVE!!!!!!!!!!!!!!!!
int MyYojimboPrintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = vprintf(fmt, args);
    va_end(args);
    return result;
}//MyYojimboPrintf

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

yojimboClient::yojimboClient(int thisDevice, char *clientAddress)
  : clientLayer(thisDevice),
    clientInstance(),
    adapter(),
    simTime(0)
{
  InitializeYojimbo();

	yojimbo::ClientServerConfig config;
	config.networkSimulator = false;

  uint8_t privateKey[yojimbo::KeyBytes];
	memset(privateKey, 0, yojimbo::KeyBytes);

  clientInstance = new yojimbo::Client( 
	yojimbo::GetDefaultAllocator(), 
	  yojimbo::Address(clientAddress), 
	  config, 
		adapter, 
		ProtocolId
	);
}//yojimboClient

bool yojimboClient::startConnection(char *address)
{
  yojimbo::Address serverAddress(address);

  uint8_t privateKey[yojimbo::KeyBytes];
	memset(privateKey, 0, yojimbo::KeyBytes);

	auto frame_interval = std::chrono::milliseconds(int(timePerFrame));
	auto next_frame_time = std::chrono::high_resolution_clock::now();
	double sim_interval = timePerFrame / 1000.0;
	double simTime = 0.0;

	clientInstance->AdvanceTime(simTime);

	clientInstance->InsecureConnect(privateKey, client, serverAddress);
	
	while(!clientInstance->IsConnected())
	{
		simTime += sim_interval;

    clientInstance->AdvanceTime(simTime);

		clientInstance->SendPackets();
		clientInstance->ReceivePackets();

		next_frame_time += frame_interval;
		std::this_thread::sleep_until(next_frame_time);
	}//while

	return true;
}//startConnection

void yojimboClient::sendMessage(json messageData)
{
  //make a new message
	jsonMessage *message = (jsonMessage*)clientInstance->CreateMessage(JSON_MESSAGE);
	//insert the required input data
	message->data = messageData;	
	//send the message
	clientInstance->SendMessage(0, message);
}//sendMessage

bool yojimboClient::receiveMessage(std::vector<json> &message)
{
  jsonMessage* newMessage = (jsonMessage*)clientInstance->ReceiveMessage(0);
  if(!newMessage)
  {
    return false;
  }//if
  if(newMessage->data.is_array())
  {
    std::vector<json> newVector;
    for(uint64_t i = 0; i <= newMessage->data.size(); i++)
    {
      newVector.push_back(newMessage->data[i]);
    }//for
    message = newVector;
  } else {
    message[0] = newMessage->data;
    message.resize(1);
  }//else
  return true;
}//receiveMessage

bool yojimboClient::startOfLoop(double simTime)
{
  clientInstance->AdvanceTime(simTime);

	clientInstance->ReceivePackets();
  return true;
}//startOfLoop

bool yojimboClient::endOfLoop()
{
  clientInstance->SendPackets();
  return true;
}//endOfLoop

void yojimboClient::endConnection()
{
	clientInstance->Disconnect();
}//endConnections

yojimboServer::yojimboServer(char *address)
  : serverInstance(),
    adapter(),
    numClients(0)
{
  InitializeYojimbo();

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
  numClients = newNumClients;
  return true;
}//startOfLoop

bool yojimboServer::endOfLoop()
{
  serverInstance->SendPackets();
  return true;
}//endOfLoop