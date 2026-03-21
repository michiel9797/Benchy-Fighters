//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "engine.h"
#include "yojimbo_layer.h"
#include "GNS_layer.h"
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
		std::cout << "./BenchyFighters SIMULATE [input player json file] [player playing on this device, 1 or 2] [IP to use] [server IP] [netcode to uses]" << std::endl;
		std::cout << "Server for simulation call:" << std::endl;
		std::cout << "./BenchyFighters SERVER [IP to use] [netcode to use]" << std::endl;
		return 0;
	}//if

	if(!((argc == 7 && exec_mode == "SIMULATE") || 
		(argc == 4 && exec_mode == "EMULATE") || 
		(argc == 4 && exec_mode == "SERVER")))
	{
		std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
		return -1;
	}//ifs

	if(exec_mode == "SERVER"){
		serverLayer* server;
		std::string netcode = argv[3];

		if(netcode == "YOJIMBO")
		{
			server = new yojimboServer(argv[2]);
		}else if(netcode == "GNS")
		{
			server = new GNSServer(argv[2]);
		}else{
			std::cerr << "Invalid netcode value" << std::endl;
			return -1;
		}//else

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

			clientLayer* client;
			std::string netcode = argv[6];

			if(netcode == "YOJIMBO")
			{
				client = new yojimboClient(currentPlayer, argv[4]);
			}else if(netcode == "GNS")
			{
				client = new GNSClient(currentPlayer);
			}else{
				std::cerr << "Invalid netcode value" << std::endl;
				return -1;
			}//else

			if(client->startConnection(argv[5]))
			{
				clientLoop(gameEngine, client, data);
			}else{
				std::cerr << "Failed to connect to server" << std::endl;
				return -1;
			}//else


			client->endConnection();

		}else{
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main