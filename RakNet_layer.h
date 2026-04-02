//RakNet_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef RakNetLayerH
#define RakNetLayerH


#include "RakNet/Source/MessageIdentifiers.h"
#include "RakNet/Source/RakPeerInterface.h"
#include "RakNet/Source/BitStream.h"
#include "RakNet/Source/RakNetTypes.h"
#include "network_layer.h"

using json = nlohmann::json;

enum messages
{
	ID_MESSAGE_1=ID_USER_PACKET_ENUM+1
};

///////////////////////////////////////////////////
// Message definition
///////////////////////////////////////////////////

class RakNetMessage: public virtual networkMessage
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
};//RakNetMessage

///////////////////////////////////////////////////
// Client definition
///////////////////////////////////////////////////

class RakNetClient: public virtual clientLayer
{
	public:
		//initialize RakNet
		RakNetClient(int thisDevice);
		//destructor
		~RakNetClient();
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
	private:
		//the client object
		RakNet::RakPeerInterface *clientInstance;
		//the target GUID
		RakNet::RakNetGUID targetGUID;
		//if there is a message in the buffer
		bool messageInBuffer;
		//a message buffer, used when checking if there are still messages
		json messageBuffer;
};

///////////////////////////////////////////////////
// Server definition
///////////////////////////////////////////////////

class RakNetServer: public virtual serverLayer
{
	public:
		//initialize RakNet
		RakNetServer(char *address);
		//deconstrutor
		~RakNetServer();
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
	
	private:
		//the server object
		RakNet::RakPeerInterface *serverInstance;
		//the target GUIDs
		RakNet::RakNetGUID targetGUID[2];

		//if the match has started yet
		bool matchStarted;
};//serverlayer

#endif