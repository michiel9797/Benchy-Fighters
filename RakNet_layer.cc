//RakNet_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <thread>

#include "RakNet_layer.h"

using json = nlohmann::json;

///////////////////////////////////////////////////
// Message code
///////////////////////////////////////////////////

RakNetMessage::RakNetMessage()
	: message(nullptr),
	  hasMessage(false)
{
	//no further initialization needed
}//RakNetMessage

RakNetMessage::RakNetMessage(const RakNet::Packet* packet)
	: message(),
	  hasMessage(true)
{	//account for data[0] offset when reading
	RakNet::BitStream bitStream(packet->data + 1, packet->length - 1, false);
	unsigned short length;
	bitStream.Read(length);

	std::string jsonString;
	jsonString.resize(length);
		
	bitStream.Read(&jsonString[0], length);

	message = json::parse(jsonString);
	if(!message.is_array())
		message = json::array({message});
}//RakNetMessage

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

RakNetClient::RakNetClient(const short thisDevice)
	: clientLayer(thisDevice),
	  clientInstance(RakNet::RakPeerInterface::GetInstance()),
	  targetGUID(RakNet::UNASSIGNED_RAKNET_GUID)
{
	//no further initialization needed
}//RakNetClient

RakNetClient::~RakNetClient()
{
	RakNet::RakPeerInterface::DestroyInstance(clientInstance);
}//~RakNetClient

bool RakNetClient::startConnection(const char *address)
{
	RakNet::SocketDescriptor descriptor = RakNet::SocketDescriptor();

	clientInstance->Startup(1, &descriptor, 1);

	//split the IP address and the port for the connect function
    const std::string addrStr(address);
	const size_t pos = addrStr.find(':');

    const std::string IP = addrStr.substr(0, pos);
    const std::string portStr = addrStr.substr(pos + 1);
	const int port = stoi(portStr);

	clientInstance->Connect(IP.c_str(), port, 0, 0);

	//wait for confirmation that we've connected to the server
	const auto frame_interval = std::chrono::milliseconds(int(timePerFrame));
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
	const std::string jsonString = messageData.dump();
	
	const uint32_t length = (uint32_t)jsonString.size();

	bitStream.Write((RakNet::MessageID)ID_MESSAGE_1);
	bitStream.Write((unsigned short)length);
	bitStream.Write(jsonString.c_str(), length);

	clientInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, targetGUID, false);
}//sendMessage

networkMessage* RakNetClient::receiveMessage()
{
	RakNetMessage* message = new RakNetMessage();

	RakNet::Packet* packet;
	for(packet = clientInstance->Receive(); packet; 
		clientInstance->DeallocatePacket(packet), packet = clientInstance->Receive())
	{	//using a switch here mostly to keep packet handling uniform
		switch(packet->data[0])
		{
			case ID_MESSAGE_1:
			{	
				delete message;
				message = new RakNetMessage(packet);
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
	return message;
}//receiveMessage

bool RakNetClient::startOfLoop(const double simTime)
{   //no actions required
	return true;
}//startOfLoop

bool RakNetClient::endOfLoop()
{   //no actions required
	return true;
}//endOfLoop

void RakNetClient::endConnection()
{
	clientInstance->Shutdown(300);
}//endConnection

///////////////////////////////////////////////////
// Server code
///////////////////////////////////////////////////

RakNetServer::RakNetServer(const char *address)
	: serverInstance(RakNet::RakPeerInterface::GetInstance()),
	  targetGUID(RakNet::UNASSIGNED_RAKNET_GUID),
	  matchStarted(false)
{
	//split the IP address and the port for the connect function
    const std::string addrStr(address);
	const size_t pos = addrStr.find(':');

    const std::string portStr = addrStr.substr(pos + 1);
	const int port = stoi(portStr);
	
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

		const std::string jsonString = message.dump();
	
		const uint32_t length = (uint32_t)jsonString.size();

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
					targetGUID[0] == RakNet::UNASSIGNED_RAKNET_GUID;
				else
					targetGUID[1] == RakNet::UNASSIGNED_RAKNET_GUID;
				break;
			case ID_MESSAGE_1:
				RakNet::BitStream bitStream(packet->data, packet->length, false);

				if(packet->guid == targetGUID[0])
					serverInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, targetGUID[1], false);
				else
					serverInstance->Send(&bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, targetGUID[0], false);
				break;
		}//switch
	}//for
}//exchangeMessages

bool RakNetServer::startOfLoop(const double simTime)
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
					targetGUID[0] = packet->guid;
				else
					targetGUID[1] = packet->guid;
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