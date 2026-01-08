//network_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 15-04-2025

#ifndef NetworkLayerH
#define NetworkLayerH

#include <vector>
#include <utility>
#include "nlohmann-json/json.hpp"
#include "yojimbo/include/yojimbo.h"

using json = nlohmann::json;

//The amount of frames the game engine should be behind simulation time
const int networkDelay = 5;

// Max message buffer size
const int maxMessageBuffer = 2048;

class networkLayer
{
	public:
		//destructor
		virtual ~networkLayer() = default;
		//a function that is called at the start of every network device loop.
		//if false is returned, the server will be shut down.
		//does not need to be overridden
		virtual bool startOfLoop(double simTime){return true;};
		//a function that is called at the end of every network device loop.
		//if false is returned, the server will be shut down.
		//does not need to be overriden
		virtual bool endOfLoop(){return true;};
};//networkingLayer

class clientLayer: public virtual networkLayer
{
	public:
		//initialize the device value to tell the client layer which device it is
		clientLayer(int thisDevice);
		//destructor
		virtual ~clientLayer() = default;
		//send a message in the shape of a json object, can be used to express
		//other messages too
		virtual void sendMessage(json message) = 0;
		//receive messages that have been sent to you. Returns true when a message
		//has been successfully loaded into the given variable, returns false otherwise
		virtual bool receiveMessage(json &message) = 0;
		//start the connection with the other device, once finished the device
		//will assume the connection has been established
		virtual bool startConnection() = 0;
		//break the connection with other devices
		virtual void endConnection() = 0;

	private:
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

class yojimboClient: public virtual clientLayer
{
	public:
		//initialize yojibmo
		yojimboClient(int thisDevice);
		//start the connection with the other device, once returned with True 
		//the device will assume the connection has been established
		bool startConnection() override;
		//send a message in the shape of a json, can be used to express
		//other messages too
		void sendMessage(json message) override;
		//receive messages that have been sent to you. Returns true when a message
		//has been successfully loaded into the given variable, returns false otherwise
		bool receiveMessage(json &message) override;
		//a function that is called at the start of every network device loop.
		bool startOfLoop(double simTime) override;
		//a function that is called at the end of every network device loop.
		bool endOfLoop() override;
		//break the connection with other devices
		void endConnection() override;
};//yojimboLayer

class yojimboServer: public virtual serverLayer
{
	public:
		//initialize yojimbo
		yojimboServer(char *address);
		//destructor
		~yojimboServer();
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
		yojimbo::Server *serverInstance;
		//how many clients are connected. If this number goes down at
		//any point, the server will shut down
		int numClients;
};//yojimboServer

struct jsonMessage : public yojimbo::Message
{	
	json data;
	
	jsonMessage();

	template<typename Stream> bool Serialize(Stream & stream);

	YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS()

};//jsonMessage

enum messageEnum
{
	JSON_MESSAGE,
	NUM_MESSAGE_TYPES
};//messageEnum

YOJIMBO_MESSAGE_FACTORY_START(YojimboMessageFactory, NUM_MESSAGE_TYPES);
YOJIMBO_DECLARE_MESSAGE_TYPE( JSON_MESSAGE, jsonMessage);
YOJIMBO_MESSAGE_FACTORY_FINISH();

class yojimboAdapter: public virtual yojimbo::Adapter
{
	public:
		yojimbo::MessageFactory * CreateMessageFactory(yojimbo::Allocator & allocator)
		{
			return YOJIMBO_NEW(allocator, YojimboMessageFactory, allocator);
		}//CreateMessageFactory
};//yojimboAdapter

#endif