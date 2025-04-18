//network_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 15-04-2025

#ifndef NetworkLayerH
#define NetworkLayerH

#include <vector>
#include <utility>
#include "alpaca/alpaca.h" 

class networkingLayer
{
	public:
		//initialize the device value to tell the networking layer which device
		//it is, does not need to be overwritten necessarily
		networkingLayer(int thisDevice);
		//start the connection with the other device, once finished the device
		//will assume the connection has been established
		virtual bool startConnection() = 0;
		//have the server device send the start of match signal
		virtual void startMatch() = 0;
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

class yojimboLayer: public networkingLayer
{
	public:
		//start the connection with the other device, once returned with True 
		//the device will assume the connection has been established
		bool startConnection();
		//have the server device send the start of match signal
		void startMatch();
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

#endif