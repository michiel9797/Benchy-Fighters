//RakNet_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef GNSLayerH
#define GNSLayerH

#include "gamenetworkingsockets/include/steam/steamnetworkingsockets.h"
#include "gamenetworkingsockets/include/steam/isteamnetworkingutils.h"
#include "network_layer.h"

using json = nlohmann::json;

///////////////////////////////////////////////////
// Message definition
///////////////////////////////////////////////////

class GNSMessage: public virtual networkMessage
{
	public:
		//get the json object inside the message
		json getJson() const override;
		//check if a message is currently loaded in
		explicit operator bool() const override;
	private:
		//the message from the package
		json message;
		//if we have a message or not
		bool hasMessage;
};//GNSMessage

///////////////////////////////////////////////////
// Client definition
///////////////////////////////////////////////////

class GNSClient: public virtual clientLayer
{
	public:
		//initialize GNS
		GNSClient(int thisDevice);
		//start the connection with the other device, once finished the device
		//will assume the connection has been established
		bool startConnection(char *address) override;
		//send a message in the shape of a json object
		void sendMessage(json messageData) override;
		//check if there are still messages to receive
		bool hasMessageToReceive() override;
		//receive messages that have been sent to you. Returns true when a message
		//has been successfully loaded into the given variable, returns false otherwise
		bool receiveMessage(json &message) override;
		//a function that is called at the start of every network device loop.
		bool startOfLoop(double simTime) override;
		//a function that is called at the end of every network device loop.
		bool endOfLoop() override;
		//break the connection with other devices
		void endConnection() override;
		//called when a connections status changes		
		void onConnectionChange(SteamNetConnectionStatusChangedCallback_t *info);
	private:
		//the client object
		ISteamNetworkingSockets *clientInstance;
		HSteamNetConnection connection;
		//if there is a message in the buffer
		bool messageInBuffer;
		//a message buffer, used when checking if there are still messages
		json messageBuffer;
		//called when a connections status changes
		static void handleConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t *info);
		inline static GNSClient *clientCallbackInstance = nullptr;
};

///////////////////////////////////////////////////
// Server definition
///////////////////////////////////////////////////

class GNSServer: public virtual serverLayer
{
	public:
		//initialize GNS
		GNSServer(char *address);
		//have the server device attempt to send the start of match signal.
		//Returns false if the match hasn't been started yet, 
		//and true if it has been started
		bool startMatch() override;
		//exchange messages between clients
		void exchangeMessages() override;
		//update the servers time and receive packets
		bool startOfLoop(double simTime) override;
		//send packets
		bool endOfLoop() override;
		//called when a connection status changes		
		void onConnectionChange(SteamNetConnectionStatusChangedCallback_t *info);
	
	private:
		//the server object
		ISteamNetworkingSockets *serverInstance;
		HSteamListenSocket listeningSocket;

		//for storing client connections
		HSteamNetConnection connections[2];

		//if the game has started
		bool gameStarted;

		//called when a connections status changes
		static void handleConnectionStatusChange(SteamNetConnectionStatusChangedCallback_t *info);
		inline static GNSServer *serverCallbackInstance = nullptr;
};//serverlayer

#endif