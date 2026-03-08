//GNS_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <thread>
#include "GNS_layer.h"

using json = nlohmann::json;

GNSClient::GNSClient(int thisDevice)
	: clientLayer(thisDevice),
	  clientInstance(SteamNetworkingSockets()),
	  connection(k_HSteamNetConnection_Invalid)
{
	//no further initialization needed
}//GNSClient

bool GNSClient::startConnection(char *address)
{
	SteamNetworkingIPAddr addr;
	addr.ParseString(address);
	SteamNetworkingConfigValue_t opt;
	connection = clientInstance->ConnectByIPAddress(addr, 1, &opt);
	if (connection == k_HSteamNetConnection_Invalid)
		return false;
	return true;
}//startConnection

void GNSClient::sendMessage(json messageData)
{
	//tbd
}//sendMessage

bool GNSClient::hasMessageToReceive()
{
	//tbd
	return false;
}//hasMessageToReceive

bool GNSClient::receiveMessage(std::vector<json> &message)
{
	//tbd
	return false;
}//receiveMessage

bool GNSClient::startOfLoop(double simTime)
{
	//tbd
	return false;
}//startOfLoop

bool GNSClient::endOfLoop()
{
	//tbd
	return false;
}//endOfLoop

void GNSClient::endConnection()
{
	//tbd
}//endConnection

GNSServer::GNSServer(char *address)
	: serverInstance(SteamNetworkingSockets()),
	  listeningSocket(),
	  connections(k_HSteamNetConnection_Invalid),
	  gameStarted(false)
{
	SteamNetworkingIPAddr serverAddress;
	serverAddress.Clear();
	uint16 addressNumber = std::stoi(address);
	serverAddress.m_port = addressNumber;

	SteamNetworkingConfigValue_t settings;
	listeningSocket = serverInstance->CreateListenSocketIP(serverAddress, 1, &settings);
}//GNSServer

bool GNSServer::startMatch()
{
	return false;
}//startMatch

void GNSServer::exchangeMessages()
{
	//tbd
}//exchangeMessages

bool GNSServer::startOfLoop(double simTime)
{
	serverInstance->RunCallbacks();
	if(gameStarted && (connections[0] == 0 || connections[1] == 0))
		return false;

	return true;
}//startOfLoop

bool GNSServer::endOfLoop()
{
	//no actions required
	return true;
}//endOfLoop