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

const uint64_t ProtocolId = 0x0123456789ABCDEFULL;

using json = nlohmann::json;


//REMOVE!!!!!!!!!!!!!!!!
int MyYojimboPrintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = vprintf(fmt, args);
    va_end(args);
    return result;
}//MyYojimboPrintf

//for grabbing the inputs to send over to the other client
json extractInputForFrame(json &data, int frame){
	//make a json object to copy inputs into
	json copy;
	//keep count of what line of the copy we're working on
	long unsigned int copyCount = 0;

	for(long unsigned int i = 0; i < data.size(); i++)
	{
		if(data[i]["Pressed"] != nullptr)
		{
			std::string timeString = data[i]["Time"];
			float time = std::stof(timeString) * 1000;

			//if the input exceeds the time limit we're looking in
			if(time >= frame * timePerFrame)
				break;

			copy[copyCount] = data[i];
			copyCount++;
			data[i]["Pressed"] = nullptr;
		}//if
	}//for
	return copy;
}//extractInputForFrame

//the main loop for an emulation run
void localLoop(engine &gameEngine)
{
	gameEngine.manageInputs(1);
	gameEngine.manageInputs(2);
	auto frame_interval = std::chrono::milliseconds(timePerFrame); 
    auto current = std::chrono::high_resolution_clock::now();
    while (!gameEngine.getFinished()) {
        gameEngine.printGamestate(gameEngine.framegen());
		gameEngine.prepStatecache();
		gameEngine.manageInputs(1);
		gameEngine.manageInputs(2);
        current += frame_interval;
		std::this_thread::sleep_until(current);
    }//while
}//localLoop

//the main loop for a client in the simulation run
void clientLoop(engine &gameEngine, yojimbo::Client &clientInstance, double simTime)
{
	auto frame_interval = std::chrono::milliseconds(timePerFrame);
	auto next_frame_time = std::chrono::high_resolution_clock::now();
	double sim_interval = timePerFrame / 1000.0;
	int frame = 0;
	bool start = false;
	
	while(1)
	{
		simTime += sim_interval;
		clientInstance.AdvanceTime(simTime);

		clientInstance.ReceivePackets();

		if (!clientInstance.IsConnected())
			break;

		jsonMessage *message = (jsonMessage*)clientInstance.ReceiveMessage(0);
		while (message)
		{
				std::cout << "Message:\n" << message->data << std::endl;
				clientInstance.ReleaseMessage(message);
				message = (jsonMessage*)clientInstance.ReceiveMessage(0);
		}//while

		//clientInstance.SendMessage(0, message);

		clientInstance.SendPackets();

		next_frame_time += frame_interval;
		std::this_thread::sleep_until(next_frame_time);
	}//while
}//clientLoop

//the main loop for a server in the simulation run
void serverLoop(yojimbo::Server &serverInstance)
{
	auto frame_interval = std::chrono::milliseconds(timePerFrame);
	auto next_frame_time = std::chrono::high_resolution_clock::now();
	double sim_interval = timePerFrame / 1000.0;
	double simTime = 0.0;
	//have two clients connected yet
	bool clientPair = false;

	while (true)
	{
		simTime += sim_interval;
		serverInstance.AdvanceTime(simTime);

		serverInstance.ReceivePackets();
		
		//if two clients havent been connected before this yet
		if(!clientPair)
		{	//if two clients are now connected
			if(serverInstance.GetNumConnectedClients() == 2)
			{
				jsonMessage *message1 = (jsonMessage*)serverInstance.CreateMessage(0, JSON_MESSAGE);
				message1->data["Start"] = "true";
				jsonMessage *message2 = (jsonMessage*)serverInstance.CreateMessage(1, JSON_MESSAGE);
				message2->data["Start"] = "true";
				serverInstance.SendMessage(0, 0, message1);
				serverInstance.SendMessage(1, 0, message2);

				clientPair = true;
			}//if
		} else {
			//tbd
		}//else

		serverInstance.SendPackets();
		
		next_frame_time += frame_interval;
		std::this_thread::sleep_until(next_frame_time);
	}//while
}//serverLoop

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
		InitializeYojimbo();
		yojimboAdapter adapter;
		yojimbo::ClientServerConfig config;
		config.networkSimulator = false;

    	uint8_t privateKey[yojimbo::KeyBytes];
    	memset( privateKey, 0, yojimbo::KeyBytes );

		yojimbo::Server serverInstance(
			yojimbo::GetDefaultAllocator(),
			privateKey,
			yojimbo::Address(argv[2]),
			config,
			adapter,
			ProtocolId
		);

		serverInstance.Start(2);

		std::cout << "SERVER START" << std::endl;
		
		char addressString[256];
		serverInstance.GetAddress().ToString( addressString, sizeof( addressString ) );
		printf( "server address is %s\n", addressString );

		yojimbo_log_level(YOJIMBO_LOG_LEVEL_INFO);
		yojimbo_set_printf_function(MyYojimboPrintf);

		serverLoop(serverInstance);

		serverInstance.Stop();
		return 0;
	} else {
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
		} else if(exec_mode == "SIMULATE")
		{
			int currentPlayer = std::stoi(argv[3]);
			if(currentPlayer >= 3)
			{
				std::cerr << "ERROR: The player value exceeds the acceptable limit" << std::endl;
				return -1;
			}
			gameEngine.setCurrentPlayer(currentPlayer);
		
			InitializeYojimbo();
			yojimboAdapter adapter;
			yojimbo::ClientServerConfig config;
			config.networkSimulator = false;

    		yojimbo::Client clientInstance( 
				yojimbo::GetDefaultAllocator(), 
				yojimbo::Address(argv[4]), 
				config, 
				adapter, 
				ProtocolId
			);

			char addressString[256];
    		clientInstance.GetAddress().ToString(addressString, sizeof(addressString));
			if(addressString[0] == 'N')
			{
				std::cerr << "ERROR: Bad address" << std::endl;
				return -1;
			}//if
			
    		std::cout << "client address is \n" << addressString << std::endl;

			yojimbo_log_level(YOJIMBO_LOG_LEVEL_DEBUG);
			yojimbo_set_printf_function(MyYojimboPrintf);

			yojimbo::Address serverAddress(argv[5]);
			uint8_t privateKey[yojimbo::KeyBytes];
   			memset(privateKey, 0, yojimbo::KeyBytes);

			auto frame_interval = std::chrono::milliseconds(timePerFrame);
			auto next_frame_time = std::chrono::high_resolution_clock::now();
			double sim_interval = timePerFrame / 1000.0;
			double simTime = 0.0;

			clientInstance.AdvanceTime(simTime);

			clientInstance.InsecureConnect(privateKey, 0, serverAddress);
			
			while(!clientInstance.IsConnected())
			{
				simTime += sim_interval;

				clientInstance.AdvanceTime(simTime);

				clientInstance.SendPackets();
				clientInstance.ReceivePackets();
				
				next_frame_time += frame_interval;
				std::this_thread::sleep_until(next_frame_time);
			}//while

			std::cout << "Connected" << std::endl;

			clientLoop(gameEngine, clientInstance, simTime);

			clientInstance.Disconnect();

		} else {
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main