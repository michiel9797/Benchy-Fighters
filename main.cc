//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 14-04-2025

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "nlohmann-json/json.hpp"
#include "engine.h"

using json = nlohmann::json;

void gameLoop(engine gameEngine)
{
	float counter = 0;
    int realcounter = 0;
	gameEngine.manageInputs(1);
	gameEngine.manageInputs(2);
    auto frame_interval = std::chrono::milliseconds(timePerFrame);
    auto start = std::chrono::high_resolution_clock::now();
    auto current = start;
	engine storeEngine;
    while (!gameEngine.getFinished()) {
        gameEngine.printGamestate(gameEngine.framegen());
		gameEngine.prepStatecache();
		gameEngine.manageInputs(1);
		gameEngine.manageInputs(2);
        current += frame_interval;
        counter += (int)timePerFrame;
        realcounter += 1;
		std::this_thread::sleep_until(current);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;
    std::cout << "Counter reached: " << counter << std::endl;
    std::cout << "Frames counted: " << realcounter << std::endl;
}//gameLoop



//arguments:
//0: program name
//1: execution mode, either "SIMULATE" or "EMULATE"
//(a simulation run is a run where another game instance has to be launched and connected)
//(an emulation run is a run where 2 players are emulated on the same device)
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
	if(exec_mode == "--help")
	{
		std::cout << "arguments:" << std::endl;
		std::cout << "0: program name" << std::endl;
		std::cout << "1: execution mode, either \"SIMULATE\" or \"EMULATE\"" << std::endl;
		std::cout << "(a simulation run is a run where another game instance has to be launched and connected)" << std::endl;
		std::cout << "(an emulation run is a run where 2 players are emulated on the same device)" << std::endl;
		std::cout << "2: json file with player input" << std::endl;
		std::cout << "|-EMULATE mode: input is routed to player 1" << std::endl;
		std::cout << "|-SIMULATE mode: input is routed to the player pointed at by argument 3" << std::endl;
		std::cout << "3:" << std::endl;
		std::cout << "|-EMULATE mode: json file with player input, is routed to player 2"<< std::endl;
		std::cout << "|-SIMULATE mode: which player is playing on this device" << std::endl;
		std::cout << "||-1: player 1 is playing on this device" << std::endl;
		std::cout << "||-2: player 2 is playing on this device" << std::endl; 
		return 0;
	}//if

	if(!(argc == 4 && (exec_mode == "SIMULATE" || exec_mode == "EMULATE")))
	{
		std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
		return -1;
	}//if
	engine gameEngine;

	std::ifstream f(argv[2]);
	json data = json::parse(f);
	//set the gamestate for the first iteration
	gameEngine.setFirstFrame();

	if(exec_mode == "EMULATE")
	{
		gameEngine.initInput(1, data);
		std::ifstream f2(argv[3]);
		data = json::parse(f2);
		if(gameEngine.initInput(2, data) == -1)
			return -1;
		gameEngine.setCurrentPlayer(-1);
		gameLoop(gameEngine);
	} else {
		int currentPlayer = std::stoi(argv[3]);
		gameEngine.initInput(currentPlayer, data);
		gameEngine.setCurrentPlayer(currentPlayer);
	}//else
	
	return 0;
}//main