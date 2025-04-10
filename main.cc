//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 10-04-2025

#include <iostream>
#include <fstream>
#include "nlohmann-json/json.hpp"
#include "engine.h"

using json = nlohmann::json;

//arguments:
//0: program name
//1: execution mode, either "SIMULATE" or "EMULATE"
//2: json file with player input
//-EMULATE: input is routed to player 1
//-SIMULATE: input is routed to the player pointed at by argument 3
//3:
//-EMULATE: json file with player input, is routed to player 2
//-SIMULATE: which player is playing on this device
//--1: player 1 is playing on this device
//--2: player 2 is playing on this device
int main(int argc, char * argv[])
{
	std::string exec_mode = argv[1];
	if(!(argc == 4 && (exec_mode == "SIMULATE" || exec_mode == "EMULATE")))
	{
		std::cerr << "Incorrect program call" << std::endl;
		return -1;
	}
	engine gameEngine;

	std::ifstream f(argv[2]);
	json data = json::parse(f);

	if(exec_mode == "EMULATE")
	{
		gameEngine.initInput(1, data);
		std::ifstream f2(argv[3]);
		data = json::parse(f2);
		gameEngine.initInput(2, data);
		gameEngine.setCurrentPlayer(-1);
	} else {
		int currentPlayer = std::stoi(argv[3]);
		gameEngine.initInput(currentPlayer, data);
		gameEngine.setCurrentPlayer(currentPlayer);
	}//else
	
	return 0;
}//main