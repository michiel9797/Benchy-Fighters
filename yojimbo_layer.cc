//yojimbo_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <thread>

#include "yojimbo_layer.h"

using json = nlohmann::json;

jsonMessage::jsonMessage()
	: data(json::array())
{
	//no further initialization needed
}//jsonMessage

template<typename Stream> bool jsonMessage::Serialize(Stream & stream)
{
	std::string jsonString;

	if(Stream::IsWriting)                                                                                    
    	jsonString = data.dump();                 
 
	//serialize the length
	size_t length = jsonString.size();

	//drop if the message is too long
	if(length > maxMessageBuffer)
    	return false;

	serialize_int(stream, length, 0, maxMessageBuffer);

	if(Stream::IsWriting)
    	for (size_t i = 0; i < length; ++i)
        	serialize_bits(stream, jsonString[i], 8);
	else{ //reading  
		std::string readString;
		readString.resize(length);

		for (size_t i = 0; i < length; ++i)
        	serialize_bits(stream, readString[i], 8);

		data = json::parse(readString); 

		//ensure the result is an array, easier later down the line
		if(!data.is_array())
			data = json::array({data});                         
	}//else        
	return true;
}//serialize

///////////////////////////////////////////////////
// Message code
///////////////////////////////////////////////////

yojimboMessage::yojimboMessage()
	: message(nullptr),
	  hasMessage(false)
{
  //no further initialization needed
}//yojimboMessage

yojimboMessage::yojimboMessage(const jsonMessage* newMessage)
	: message(nullptr),
	  hasMessage()
{
	if(newMessage)
	{
		message = newMessage->data;
		hasMessage = true;
	}else
		hasMessage = false;
}//yojimboMessage

json yojimboMessage::getJson() const
{
	return message;
}//getJson

yojimboMessage::operator bool() const
{
	return hasMessage;
}//bool

///////////////////////////////////////////////////
// Client code
///////////////////////////////////////////////////

yojimboClient::yojimboClient(const short thisDevice, const char *clientAddress)
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

bool yojimboClient::startConnection(const char *address)
{
	yojimbo::Address serverAddress(address);

	uint8_t privateKey[yojimbo::KeyBytes];
	memset(privateKey, 0, yojimbo::KeyBytes);

	const auto frame_interval = std::chrono::milliseconds(int(timePerFrame));
	auto next_frame_time = std::chrono::high_resolution_clock::now();
	const double sim_interval = timePerFrame / 1000.0;

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
{	//make a new message
	jsonMessage *message = (jsonMessage*)clientInstance->CreateMessage(JSON_MESSAGE);
	//insert the required input data
	message->data = messageData;
	//send the message
	clientInstance->SendMessage(0, message);
}//sendMessage

networkMessage* yojimboClient::receiveMessage()
{
	jsonMessage* temp = (jsonMessage*)clientInstance->ReceiveMessage(0);
	yojimboMessage* message = new yojimboMessage(temp);
	if(temp)
		clientInstance->ReleaseMessage(temp);
	return message;
}//receiveMessage

bool yojimboClient::startOfLoop(const double simInterval)
{
	simTime += simInterval;
	clientInstance->AdvanceTime(simTime);

	if(!clientInstance->IsConnected())
		return false;

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

///////////////////////////////////////////////////
// Server code
///////////////////////////////////////////////////

yojimboServer::yojimboServer(const char *address)
	: serverInstance(),
	  adapter(),
	  numClients(0),
      simTime(0)
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
{ 	//if two clients are connected
	if(numClients == 2)
	{	//generate and send the start message
		jsonMessage *message1 = (jsonMessage*)serverInstance->CreateMessage(0, JSON_MESSAGE);
		message1->data[0]["Start"] = "true";
		jsonMessage *message2 = (jsonMessage*)serverInstance->CreateMessage(1, JSON_MESSAGE);
		message2->data[0]["Start"] = "true";
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

bool yojimboServer::startOfLoop(const double simInterval)
{
	simTime += simInterval;
	serverInstance->AdvanceTime(simTime);
	serverInstance->ReceivePackets();
	int newNumClients = serverInstance->GetNumConnectedClients();
	if(newNumClients < numClients)
		return false;

	numClients = newNumClients;
	return true;
}//startOfLoop

bool yojimboServer::endOfLoop()
{
	serverInstance->SendPackets();
	return true;
}//endOfLoop