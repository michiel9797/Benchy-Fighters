//network_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef NetworkLayerH
#define NetworkLayerH

#include <vector>
#include "nlohmann-json/json.hpp"
#include "engine.h"

const uint64_t ProtocolId = 0x0123456789ABCDEFULL;

using json = nlohmann::json;

//The amount of frames the game engine should be behind simulation time
const int networkDelay = 5;

//Max message buffer size
const int maxMessageBuffer = 2048;

class networkMessage
{
	public:
		//destructor
		virtual ~networkMessage() = default;
		//get the json object inside the message
		virtual json getJson() const = 0;
		//check if a message is currently loaded in
		virtual explicit operator bool() const = 0;
};//benchyMessage

class networkLayer
{
	public:
		//destructor
		virtual ~networkLayer() = default;
		//a function that is called at the start of every network device loop.
		//if false is returned, the device will be shut down.
		//does not need to be overridden
		virtual bool startOfLoop(double simTime){return true;};
		//a function that is called at the end of every network device loop.
		//if false is returned, the device will be shut down.
		//does not need to be overriden
		virtual bool endOfLoop(){return true;};
};//networkingLayer

//note: the game will automatically shut down a client once a player reaches 0
//health or 99 seconds worth of frames have been generated, but the client can
//also be shut down by use of the startOfLoop or endOfLoop functions.
class clientLayer: public virtual networkLayer
{
	public:
		//initialize the device value to tell the client layer which device it is
		clientLayer(int thisDevice);
		//destructor
		virtual ~clientLayer() = default;
		//send a message in the shape of a json object
		virtual void sendMessage(json messageData) = 0;
		//check if there are still messages to receive
		virtual bool hasMessageToReceive() = 0;
		//receive messages that have been sent to you. Returns true when a message
		//has been successfully loaded into the given variable, returns false otherwise
		virtual bool receiveMessage(json &message) = 0;
		//start the connection with the other device, once finished the device
		//will assume the connection has been established
		virtual bool startConnection(char *address) = 0;
		//break the connection with other devices
		virtual void endConnection() = 0;

	protected:
		//which client we are
		int client;
};

//note: there is no explicit function called to check if a server needs
//to be shut down. This is expected to be handled in either the startOfLoop
//or endOfLoop function.
class serverLayer: public virtual networkLayer
{
	public:
		//destructor
		virtual ~serverLayer() = default;
		//have the server device attempt to send the start of match signal.
		//Returns false if the match hasn't been started yet, 
		//and true if it has been started
		virtual bool startMatch() = 0;
		//exchange messages between clients
		virtual void exchangeMessages() = 0;
};//serverlayer

#endif