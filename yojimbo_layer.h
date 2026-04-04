//yojimbo_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef YojimboLayerH
#define YojimboLayerH

#include "network_layer.h"
#include "yojimbo/include/yojimbo.h"

using json = nlohmann::json;

struct jsonMessage: public yojimbo::Message
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

///////////////////////////////////////////////////
// Message definition
///////////////////////////////////////////////////

class yojimboMessage: public virtual networkMessage
{
	public:
		//base constructor
		yojimboMessage();
		//constructor that loads in the jsonMessage
		yojimboMessage(jsonMessage* newMessage);
		//get the json object inside the message
		json getJson() const override;
		//check if a message is currently loaded in
		explicit operator bool() const override;
	private:
		//the message from the package
		json message;
		//if we have a message or not
		bool hasMessage;
};//yojimboMessage

///////////////////////////////////////////////////
// Client definition
///////////////////////////////////////////////////

class yojimboClient: public virtual clientLayer
{
	public:
		//initialize yojibmo
		yojimboClient(int thisDevice, char *clientAddress);
		//start the connection with the other device, once returned with True 
		//the device will assume the connection has been established
		bool startConnection(char *serverAddress) override;
		//send a message in the shape of a json, can be used to express
		//other messages too
		void sendMessage(json message) override;
		//receive messages that have been sent to you
		networkMessage* receiveMessage() override;
		//a function that is called at the start of every network device loop.
		bool startOfLoop(double simTime) override;
		//a function that is called at the end of every network device loop.
		bool endOfLoop() override;
		//break the connection with other devices
		void endConnection() override;
	private:
		//the client object
		yojimbo::Client *clientInstance;
		//adapter object
		yojimboAdapter adapter;
		//simulation time, to ensure time stays absolute between contexts
		double simTime;
};//yojimboLayer

///////////////////////////////////////////////////
// Server definition
///////////////////////////////////////////////////

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
		//adapter object
		yojimboAdapter adapter;
		//how many clients are connected. If this number goes down at
		//any point, the server will shut down
		int numClients;
		//simulation time, to ensure time stays absolute between contexts
		double simTime;
};//yojimboServer

#endif