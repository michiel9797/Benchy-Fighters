//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 30-07-2025

#include "network_layer.h"

bool yojimboLayer::startConnection()
{
	return false;
}//startConnection

void yojimboLayer::sendMessage(std::vector<std::pair<char, float>> input)
{
	return;
}//sendMessage

std::vector<std::pair<char, float>> yojimboLayer::receiveMessage()
{
	std::vector<std::pair<char, float>> temp;
	return temp;
}//receiveMessage

void yojimboLayer::endConnection()
{
	return;
}//endConnections

void yojimboServer::startMatch()
{
	return;
}//startMatch

void yojimboServer::endMatch()
{
	return;
}//endMatch