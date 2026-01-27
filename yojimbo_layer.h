//network_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef YojimboLayerH
#define YojimboLayerH

#include "network_layer.h"
#include "yojimbo/include/yojimbo.h"

using json = nlohmann::json;

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
		//check if there are still messages to receive
		bool hasMessageToReceive() override;
		//receive messages that have been sent to you. Returns true when a message
		//has been successfully loaded into the given variable, returns false otherwise
		bool receiveMessage(std::vector<json> &message) override;
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
		//if there is a message in the buffer
		bool messageInBuffer;
		//a message buffer, used when checking if there are still messages
		json messageBuffer;
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
		//adapter object
		yojimboAdapter adapter;
		//how many clients are connected. If this number goes down at
		//any point, the server will shut down
		int numClients;
		//simulation time, to ensure time stays absolute between contexts
		double simTime;
};//yojimboServer

#endif