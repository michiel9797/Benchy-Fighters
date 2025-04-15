//network_layer.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 15-04-2025

#ifndef NetworkLayerH
#define NetworkLayerH

#include <vector>
#include <utility>

class networkingLayer
{
	public:
		//initialize the device value to tell the networking layer which device
		//it is, does not need to be overwritten necessarily
		networkingLayer(int thisDevice);
		//start the connection with the other device, once finished the device
		//will assume the connection has been established
		virtual bool startConnection();
		//have the server device send the start of match signal
		virtual void startMatch();
		//send a message in the shape of an input vector, can be used to express
		//other messages too
		virtual void sendMessage(std::vector<std::pair<char, float>> input);
		//receive messages that have been sent to you [may need to be a different shape]
		virtual std::vector<std::pair<char, float>> receiveMessage();
		//break the connection with other devices
		virtual void endConnection();
	private:
		//which device we are
		int device;
};//networkingLayer

#endif