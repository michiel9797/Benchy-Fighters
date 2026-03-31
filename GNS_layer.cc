//GNS_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <thread>
#include "GNS_layer.h"

using json = nlohmann::json;

///////////////////////////////////////////////////
// Client code
///////////////////////////////////////////////////

GNSClient::GNSClient(int thisDevice)
	: clientLayer(thisDevice),
	  clientInstance(),
	  connection(k_HSteamNetConnection_Invalid),
	  messageInBuffer(false),
	  messageBuffer()
{
	SteamDatagramErrMsg errorMessage;
	GameNetworkingSockets_Init(nullptr, errorMessage);

	clientInstance = SteamNetworkingSockets();

	clientCallbackInstance = this;
}//GNSClient

bool GNSClient::startConnection(char *address)
{
	SteamNetworkingIPAddr serverAddress;
	serverAddress.Clear();
	serverAddress.ParseString(address);

	SteamNetworkingConfigValue_t settings;
	connection = clientInstance->ConnectByIPAddress(serverAddress, 0, &settings);
	if (connection == k_HSteamNetConnection_Invalid)
		return false;

	return true;
}//startConnection

void GNSClient::sendMessage(json messageData)
{
	std::string message = messageData.dump();
	SteamNetworkingSockets()->SendMessageToConnection(
		connection,
		message.data(),
		message.size(),
		k_nSteamNetworkingSend_Reliable,
		nullptr
	);
}//sendMessage

bool GNSClient::hasMessageToReceive()
{
	if(messageInBuffer)
    	return true;

	ISteamNetworkingMessage *messagePointer = nullptr;
	int messageAmount = SteamNetworkingSockets()->ReceiveMessagesOnConnection(
		connection,
		&messagePointer,
		1
	);

	if(messageAmount <= 0)
		return false;

	std::string messageData(
		(char*)messagePointer->m_pData,
		messagePointer->m_cbSize
	);

	messagePointer->Release();

	messageBuffer = json::parse(messageData);
    messageInBuffer = true;

  	return true;
}//hasMessageToReceive

bool GNSClient::receiveMessage(std::vector<json> &message)
{
	json dataReceived;

	if(messageInBuffer)
	{
		dataReceived = messageBuffer;
		messageInBuffer = false;
	}else{
		ISteamNetworkingMessage *messagePointer = nullptr;
		int messageAmount = SteamNetworkingSockets()->ReceiveMessagesOnConnection(
			connection,
			&messagePointer,
			1
		);

		if(messageAmount <= 0)
			return false;

		std::string messageData(
			(char*)messagePointer->m_pData,
			messagePointer->m_cbSize
		);

		dataReceived = json::parse(messageData);
		messagePointer->Release();
	}//else

 	if(dataReceived.is_array())
  	{
    	std::vector<json> newVector;
    	for(size_t i = 0; i < dataReceived.size(); i++)
      		newVector.push_back(dataReceived[i]);

    	message = newVector;
  	}else{
    	if(message.size() == 0)
    	{
      		message.push_back(dataReceived);
    	}else{
      		message[0] = dataReceived;
      		message.resize(1);
    	}//else
  	}//else

	return true;
}//receiveMessage

bool GNSClient::startOfLoop(double simTime)
{
	clientInstance->RunCallbacks();
	return true;
}//startOfLoop

bool GNSClient::endOfLoop()
{
	//no actions required
	return true;
}//endOfLoop

void GNSClient::endConnection()
{
	if(connection != k_HSteamNetConnection_Invalid)
	{
		SteamNetworkingSockets()->CloseConnection(
			connection,
			0,
			"",
			true
		);
		connection = k_HSteamNetConnection_Invalid;
	}//if
}//endConnection

void GNSClient::handleConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t *info)
{
	clientCallbackInstance->onConnectionChange(info);
}//handleConnectionStatusChange

void GNSClient::onConnectionChange(SteamNetConnectionStatusChangedCallback_t *info)
{
	switch (info->m_info.m_eState)
    {
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			SteamNetworkingSockets()->CloseConnection(
				info->m_hConn, 0, nullptr, false);
			break;
		default:
			//nothing to be done
			break;
    }//switch
}//handleConnectionStatusChange

///////////////////////////////////////////////////
// Server code
///////////////////////////////////////////////////

GNSServer::GNSServer(char *address)
	: serverInstance(),
	  listeningSocket(),
	  connections(k_HSteamNetConnection_Invalid),
	  gameStarted(false)
{
	SteamDatagramErrMsg errorMessage;
	GameNetworkingSockets_Init(nullptr, errorMessage);

	serverInstance = SteamNetworkingSockets();

	//set the callback function
	serverCallbackInstance = this;
	SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(&GNSServer::handleConnectionStatusChange);

	SteamNetworkingIPAddr serverAddress;
	serverAddress.Clear();
	serverAddress.ParseString(address);

	SteamNetworkingConfigValue_t settings;
	listeningSocket = serverInstance->CreateListenSocketIP(serverAddress, 0, &settings);
}//GNSServer

bool GNSServer::startMatch()
{
	if(connections[0] != k_HSteamNetConnection_Invalid && 
	   connections[1] != k_HSteamNetConnection_Invalid)
	{
		json messageData = json::object();
		messageData["Start"] = "true";
		std::string message = messageData.dump();

		for(int i = 0; i <= 1; i++)
		{
			SteamNetworkingSockets()->SendMessageToConnection(
				connections[i],
				message.data(),
				message.size(),
				k_nSteamNetworkingSend_Reliable,
				nullptr
			);
		}//for
		gameStarted = true;
		return true;
	}//if
	
	return false;
}//startMatch

void GNSServer::exchangeMessages()
{
	ISteamNetworkingMessage *messagePointer = nullptr;
    
	while(SteamNetworkingSockets()->ReceiveMessagesOnConnection(connections[0], &messagePointer, 1) > 0)
    {
       	SteamNetworkingSockets()->SendMessageToConnection(
			connections[1],
			messagePointer->m_pData,
			messagePointer->m_cbSize,
			k_nSteamNetworkingSend_Reliable,
			nullptr
		);
        messagePointer->Release();
    }//while

	while(SteamNetworkingSockets()->ReceiveMessagesOnConnection(connections[1], &messagePointer, 1) > 0)
    {
       	SteamNetworkingSockets()->SendMessageToConnection(
			connections[0],
			messagePointer->m_pData,
			messagePointer->m_cbSize,
			k_nSteamNetworkingSend_Reliable,
			nullptr
		);
        messagePointer->Release();
    }//while
}//exchangeMessages

bool GNSServer::startOfLoop(double simTime)
{
	serverInstance->RunCallbacks();
	if(gameStarted && (connections[0] == k_HSteamNetConnection_Invalid || 
					   connections[1] == k_HSteamNetConnection_Invalid))
		return false;

	return true;
}//startOfLoop

bool GNSServer::endOfLoop()
{
	//no actions required
	return true;
}//endOfLoop

void GNSServer::handleConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t *info)
{
	serverCallbackInstance->onConnectionChange(info);
}//handleConnectionStatusChange

void GNSServer::onConnectionChange(SteamNetConnectionStatusChangedCallback_t *info)
{
	switch (info->m_info.m_eState)
    {
        case k_ESteamNetworkingConnectionState_Connecting:
        	// Accept connection
            SteamNetworkingSockets()->AcceptConnection(info->m_hConn);

            if(connections[0] == k_HSteamNetConnection_Invalid)
			{
                connections[0] = info->m_hConn;
			}else
                connections[1] = info->m_hConn;

            break;
		case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
            if(info->m_hConn == connections[0])
			{
                connections[0] = k_HSteamNetConnection_Invalid;
			}else
                connections[1] = k_HSteamNetConnection_Invalid;

			//break connection
            SteamNetworkingSockets()->CloseConnection(info->m_hConn, 0, nullptr, false);
            break;
		default:
			//nothing to be done
			break;
    }//switch
}//handleConnectionStatusChange