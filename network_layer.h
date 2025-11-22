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
		//initialize the device value to tell the networking layer which device
		//it is, does not need to be overwritten necessarily
		networkLayer(int thisDevice){device = thisDevice;};
		//deconstructor
		virtual ~networkLayer();
		//start the connection with the other device, once finished the device
		//will assume the connection has been established
		virtual bool startConnection() = 0;
		//send a message in the shape of an input vector, can be used to express
		//other messages too
		virtual void sendMessage(std::vector<std::pair<char, float>> input) = 0;
		//receive messages that have been sent to you [may need to be a different shape]
		virtual std::vector<std::pair<char, float>> receiveMessage() = 0;
		//break the connection with other devices
		virtual void endConnection() = 0;
	private:
		//which device we are
		int device;
};//networkingLayer

class serverLayer: public virtual networkLayer
{
	public:
		//have the server device send the start of match signal
		virtual void startMatch() = 0;
		//have the sever device send the end of match signal
		virtual void endMatch() = 0;
};//serverlayer

class yojimboLayer: public virtual networkLayer
{
	public:
		//start the connection with the other device, once returned with True 
		//the device will assume the connection has been established
		bool startConnection();
		//send a message in the shape of an input vector, can be used to express
		//other messages too
		void sendMessage(std::vector<std::pair<char, float>> input);
		//receive messages that have been sent to you [may need to be a different shape]
		std::vector<std::pair<char, float>> receiveMessage();
		//break the connection with other devices
		void endConnection();
	private:
		//which device we are
		int device;
};//yojimboLayer

class yojimboServer: public virtual serverLayer
{
	public:
		//start the connection with the other device, once returned with True 
		//the device will assume the connection has been established
		bool startConnection();
		//have the server device send the start of match signal
		void startMatch();
		//have the sever device send the end of match signal
		virtual void endMatch();
		//send a message in the shape of an input vector, can be used to express
		//other messages too
		void sendMessage(std::vector<std::pair<char, float>> input);
		//receive messages that have been sent to you [may need to be a different shape]
		std::vector<std::pair<char, float>> receiveMessage();
		//break the connection with other devices
		void endConnection();
	private:
		//which device we are
		int device;
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