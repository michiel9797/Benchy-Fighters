//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 14-04-2025

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "nlohmann-json/json.hpp"
#include "yojimbo/include/yojimbo.h"
#include "engine.h"
#include "network_layer.h"

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
	/* Lines used for testing framerate
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration<double, std::milli>(diff).count() << " ms" << std::endl;
    std::cout << "Counter reached: " << counter << std::endl;
    std::cout << "Frames counted: " << realcounter << std::endl;
	*/
}//gameLoop


int main(int argc, char * argv[])
{
	std::string exec_mode = argv[1];
	if(exec_mode == "--help")
	{
		std::cout << "Match emulation call:" << std::endl;
		std::cout << "./BenchyFighters EMULATE [input player 1 json file] [input player 2 json file]" << std::endl;
		std::cout << "Match simulation call:" << std::endl;
		std::cout << "./BenchyFighters SIMULATE [input player json file] [player playing on this device, 1 or 2] [port to use]" << std::endl;
		std::cout << "Server for simulation call:" << std::endl;
		std::cout << "./BenchyFighters SERVER [port to use]" << std::endl;
		return 0;
	}//if

	if(!(((argc == 4 && (exec_mode == "SIMULATE" || exec_mode == "EMULATE"))) || (argc == 2 && (exec_mode == "SERVER"))))
	{
		std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
		return -1;
	}//if

	if(exec_mode == "SERVER"){
		InitializeYojimbo();
		yojimboAdapter adapter;
		yojimbo::ClientServerConfig config;

		yojimbo::Server serverInstance(
			yojimbo::GetDefaultAllocator(),
			0,
			yojimbo::Address(argv[3]),
			config,
			adapter,
			0
		);
		return 0;
	} else {
		engine gameEngine;

		std::ifstream f(argv[2]);
		json data = json::parse(f);
		//set the gamestate for the first iteration
		gameEngine.setFirstFrame();

		if(exec_mode == "EMULATE")
		{
			gameEngine.addInput(1, data);
			std::ifstream f2(argv[3]);
			data = json::parse(f2);
			if(gameEngine.addInput(2, data) == -1)
				return -1;
			gameEngine.setCurrentPlayer(-1);
			gameLoop(gameEngine);
		} else if(exec_mode == "SIMULATE")
		{
			int currentPlayer = std::stoi(argv[3]);
			gameEngine.addInput(currentPlayer, data);
			gameEngine.setCurrentPlayer(currentPlayer);
		} else {
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main