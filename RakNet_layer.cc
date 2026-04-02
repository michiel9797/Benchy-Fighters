//RakNet_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <thread>
#include "RakNet_layer.h"

#include <iostream> //!!!REMOVE!!!

using json = nlohmann::json;

///////////////////////////////////////////////////
// Message code
///////////////////////////////////////////////////

json RakNetMessage::getJson() const
{
  return message;
}//getJson

RakNetMessage::operator bool() const
{
  return hasMessage;
}//bool

///////////////////////////////////////////////////
// Client code
///////////////////////////////////////////////////

RakNetClient::RakNetClient(int thisDevice)
	: clientLayer(thisDevice),
	  clientInstance(RakNet::RakPeerInterface::GetInstance()),
	  targetGUID(RakNet::UNASSIGNED_RAKNET_GUID),
	  messageInBuffer(false),
	  messageBuffer()
{
	//no further initialization needed
}//RakNetClient

RakNetClient::~RakNetClient()
{
	RakNet::RakPeerInterface::DestroyInstance(clientInstance);
}//~RakNetClient

bool RakNetClient::startConnection(char *address)
{
	RakNet::SocketDescriptor descriptor = RakNet::SocketDescriptor();

	clientInstance->Startup(1, &descriptor, 1);

	//split the IP address and the port for the connect function
	const char split = ':';
	const char* splitPointer = &split;

	char* IP = strtok(address, splitPointer);
	char* portChar = strtok(NULL, splitPointer);
	int port = atoi(portChar);

	clientInstance->Connect(IP, port, 0, 0);

	//wait for confirmation that we've connected to the server
	auto frame_interval = std::chrono::milliseconds(int(timePerFrame));
	auto next_frame_time = std::chrono::high_resolution_clock::now();

	while(true)
	{
		RakNet::Packet *packet;
		for(packet = clientInstance->Receive(); packet; 
		    clientInstance->DeallocatePacket(packet), packet = clientInstance->Receive())
		{
			switch(packet->data[0])
			{
				case ID_CONNECTION_ATTEMPT_FAILED:
					return false;
				case ID_CONNECTION_REQUEST_ACCEPTED:
					targetGUID = packet->guid;
					return true;
			}//switch
		}//for
		next_frame_time += frame_interval;
		std::this_thread::sleep_until(next_frame_time);
	}//while
}//startConnection

void RakNetClient::sendMessage(json messageData)
{
	RakNet::BitStream bitStream;
	std::string jsonString = messageData.dump();
	
	uint32_t length = (uint32_t)jsonString.size();

	bitStream.Write((RakNet::MessageID)ID_MESSAGE_1);
	bitStream.Write((unsigned short)length);
	bitStream.Write(jsonString.c_str(), length);

	clientInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, targetGUID, false);
}//sendMessage

bool RakNetClient::hasMessageToReceive()
{
	if(messageInBuffer)
	{
		return true;
	}else{
		RakNet::Packet *packet;
		for(packet = clientInstance->Receive(); packet; 
		    clientInstance->DeallocatePacket(packet), packet = clientInstance->Receive())
		{	//using a switch here mostly to keep packet handling uniform
			switch(packet->data[0])
			{	//account for data[0] offset when reading
				case ID_MESSAGE_1:
					RakNet::BitStream bitStream(packet->data + 1, packet->length - 1, false);
					unsigned short length;
					bitStream.Read(length);

					std::string jsonString;
					jsonString.resize(length);
						
					bitStream.Read(&jsonString[0], length);

					messageBuffer = json::parse(jsonString);
					if(!messageBuffer.is_array())
						messageBuffer = json::array({messageBuffer});
					messageInBuffer = true;

					return true;
			}//switch
		}//for
	}//else
	return false;
}//hasMessageToReceive

bool RakNetClient::receiveMessage(json &message)
{
	json dataReceived = nullptr;

	if(messageInBuffer)
	{
		dataReceived = messageBuffer;
		messageInBuffer = false;
	}else{
		RakNet::Packet *packet;
		for(packet = clientInstance->Receive(); packet; 
		    clientInstance->DeallocatePacket(packet), packet = clientInstance->Receive())
		{	//using a switch here mostly to keep packet handling uniform
			switch(packet->data[0])
			{
				case ID_MESSAGE_1:
				{	//account for data[0] offset when reading
					RakNet::BitStream bitStream(packet->data + 1, packet->length - 1, false);
					unsigned short length;
					bitStream.Read(length);

					std::string jsonString;
					jsonString.resize(length);
						
					bitStream.Read(&jsonString[0], length);

					dataReceived = json::parse(jsonString);
					break;
				}//case
				default:
					continue;
			}//switch
			//in case we recieve a message, we break out of the switch and
			//subsequently break out of the loop here. In the default case,
			//we directly continue and skip this break, staying in the loop
			break;
		}//for
	}//else

	if(dataReceived.is_null())
		return false;
	
	message = dataReceived;

	if(!message.is_array())
		message = json::array({message});

	return true;
}//sendMessage

bool RakNetClient::startOfLoop(double simTime)
{
	//no actions required
	return true;
}//startOfLoop

bool RakNetClient::endOfLoop()
{
	//no actions required
	return true;
}//endOfLoop

void RakNetClient::endConnection()
{
	clientInstance->Shutdown(300);
}//endConnection

///////////////////////////////////////////////////
// Server code
///////////////////////////////////////////////////

RakNetServer::RakNetServer(char *address)
	: serverInstance(RakNet::RakPeerInterface::GetInstance()),
	  targetGUID(RakNet::UNASSIGNED_RAKNET_GUID),
	  matchStarted(false)
{
	//split the IP address and the port for the connect function
	const char split = ':';
	const char* splitPointer = &split;

	char* IP = strtok(address, splitPointer);
	char* portChar = strtok(NULL, splitPointer);
	int port = atoi(portChar);
	
	RakNet::SocketDescriptor descriptor = RakNet::SocketDescriptor(port, 0);
	serverInstance->Startup(2, &descriptor, 1);
	serverInstance->SetMaximumIncomingConnections(2);
}//RakNetServer

RakNetServer::~RakNetServer()
{
	serverInstance->Shutdown(0);
	RakNet::RakPeerInterface::DestroyInstance(serverInstance);
}//~RakNetServer

bool RakNetServer::startMatch()
{	
	if(!matchStarted && targetGUID[0] != RakNet::UNASSIGNED_RAKNET_GUID && targetGUID[1] != RakNet::UNASSIGNED_RAKNET_GUID)
	{
		RakNet::BitStream bitStream;

		json message = json::array();
		message[0]["Start"] = "true";

		std::string jsonString = message.dump();
	
		uint32_t length = (uint32_t)jsonString.size();

		bitStream.Write((RakNet::MessageID)ID_MESSAGE_1);
		bitStream.Write((unsigned short)length);
		bitStream.Write(jsonString.c_str(), length);

		serverInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_RAKNET_GUID, true);

		matchStarted = true;
		return true;
	}//if
	return false;
}//startMatch

void RakNetServer::exchangeMessages()
{
	RakNet::Packet *packet;
	for(packet = serverInstance->Receive(); packet; 
		serverInstance->DeallocatePacket(packet), packet = serverInstance->Receive())
	{
		switch(packet->data[0])
		{
			//catch disconnects for later down the line
			case ID_DISCONNECTION_NOTIFICATION:
			case ID_CONNECTION_LOST:
				if(packet->guid == targetGUID[0])
				{
					targetGUID[0] == RakNet::UNASSIGNED_RAKNET_GUID;
				}else{
					targetGUID[1] == RakNet::UNASSIGNED_RAKNET_GUID;
				}//else
				break;
			case ID_MESSAGE_1:
				RakNet::BitStream bitStream(packet->data, packet->length, false);

				if(packet->guid == targetGUID[0])
				{
					serverInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, targetGUID[1], false);
				}else{
					serverInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, targetGUID[0], false);
				}//else
				break;
		}//switch
	}//for
}//exchangeMessages

bool RakNetServer::startOfLoop(double simTime)
{
	if(!matchStarted)
	{
		RakNet::Packet* packet;
		for(packet = serverInstance->Receive(); packet; 
		    serverInstance->DeallocatePacket(packet), packet = serverInstance->Receive())
		{
			if(packet->data[0] == ID_NEW_INCOMING_CONNECTION)
			{
				if(targetGUID[0] == RakNet::UNASSIGNED_RAKNET_GUID)
				{
					targetGUID[0] = packet->guid;
				}else{
					targetGUID[1] = packet->guid;
				}//else
			}//if
		}//for
	}//if
	return true;
}//startOfLoop

bool RakNetServer::endOfLoop()
{
	if(matchStarted && (targetGUID[0] == RakNet::UNASSIGNED_RAKNET_GUID || 
					    targetGUID[1] == RakNet::UNASSIGNED_RAKNET_GUID))
		return false;
	return true;
}//endOfLoop