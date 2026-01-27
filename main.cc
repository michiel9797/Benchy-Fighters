//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "engine.h"
#include "yojimbo_layer.h"
#include "game_loop.h"

using json = nlohmann::json;

int main(int argc, char * argv[])
{
	if(argc <= 1)
	{
		std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
		return -1;
	}//if

	std::string exec_mode = argv[1];
	if(exec_mode == "--help")
	{
		std::cout << "Match emulation call:" << std::endl;
		std::cout << "./BenchyFighters EMULATE [input player 1 json file] [input player 2 json file]" << std::endl;
		std::cout << "Match simulation call:" << std::endl;
		std::cout << "./BenchyFighters SIMULATE [input player json file] [player playing on this device, 1 or 2] [IP to use] [server IP]" << std::endl;
		std::cout << "Server for simulation call:" << std::endl;
		std::cout << "./BenchyFighters SERVER [IP to use]" << std::endl;
		return 0;
	}//if

	if(!((argc == 6 && exec_mode == "SIMULATE") || 
		(argc == 4 && exec_mode == "EMULATE") || 
		(argc == 3 && exec_mode == "SERVER")))
	{
		std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
		return -1;
	}//if

	if(exec_mode == "SERVER"){
		yojimboServer server = yojimboServer(argv[2]);

		std::cout << "SERVER START" << std::endl;

		serverLoop(server);

		return 0;
	}else{
		engine gameEngine;

		std::ifstream f(argv[2]);
		json data;

		try
		{
			data = json::parse(f);
		}//try
		catch(...)
		{
			std::cerr << "ERROR: Player input provided does not specify a JSON file" << std::endl;
			return -1;
		}//catch

		//set the gamestate for the first iteration
		gameEngine.setFirstFrame();

		if(exec_mode == "EMULATE")
		{
			gameEngine.addInput(1, data);
			std::ifstream f2(argv[3]);
			try
			{
				data = json::parse(f2);
			}//try
			catch(...)
			{
				std::cerr << "ERROR: Player 2 input provided does not specify a JSON file" << std::endl;
				return -1;
			}//catch
			gameEngine.addInput(2, data);
			gameEngine.setCurrentPlayer(-1);

			std::cout << "EMULATE START" << std::endl;
			localLoop(gameEngine);
		}else if(exec_mode == "SIMULATE"){
			int currentPlayer = std::stoi(argv[3]);
			if(currentPlayer >= 3)
			{
				std::cerr << "ERROR: The player value exceeds the acceptable limit" << std::endl;
				return -1;
			}//if
			gameEngine.addInput(currentPlayer, data);
			gameEngine.setCurrentPlayer(currentPlayer);
		
			yojimboClient client = yojimboClient(currentPlayer, argv[4]);

			if(client.startConnection(argv[5]))
				clientLoop(gameEngine, client, data);

			client.endConnection();

		}else{
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main