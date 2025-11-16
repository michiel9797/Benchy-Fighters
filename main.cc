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
}

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

		auto frame_interval = std::chrono::milliseconds(timePerFrame);
		auto next_frame_time = std::chrono::high_resolution_clock::now();
		auto start_time = next_frame_time;
		double time = 0.0;

		while (true)
		{
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> total_elapsed = now - start_time;
			time = total_elapsed.count();

			serverInstance.AdvanceTime(time);
			serverInstance.ReceivePackets();
			
			if(serverInstance.IsClientConnected(0))
			{
				jsonMessage *message = (jsonMessage*)serverInstance.ReceiveMessage(0, 0);
				while (message)
				{
					std::cout << "Message:\n" << message->data << std::endl;
					serverInstance.ReleaseMessage(0, message);
					message = (jsonMessage*)serverInstance.ReceiveMessage(0, 0);
				}//while
			}//if

			serverInstance.SendPackets();
			
			next_frame_time += frame_interval;
			std::this_thread::sleep_until(next_frame_time);
		}//while

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
			std::cout << "ERROR: Player input provided does not specify a JSON file" << std::endl;
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
				std::cout << "ERROR: Player 2 input provided does not specify a JSON file" << std::endl;
				return -1;
			}//catch
			if(gameEngine.addInput(2, data) == -1)
				return -1;
			gameEngine.setCurrentPlayer(-1);
			std::cout << "EMULATE START" << std::endl;
			gameLoop(gameEngine);
		} else if(exec_mode == "SIMULATE")
		{
			int currentPlayer = std::stoi(argv[3]);
			if(currentPlayer >= 3)
			{
				std::cout << "ERROR: The player value exceeds the acceptable limit" << std::endl;
				return -1;
			}
			gameEngine.addInput(currentPlayer, data);
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
			auto start_time = next_frame_time;
			double time = 0.0;

			clientInstance.AdvanceTime(time);

			clientInstance.InsecureConnect(privateKey, 0, serverAddress);
			
			while(!clientInstance.IsConnected())
			{
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> total_elapsed = now - start_time;
				time = total_elapsed.count();

				clientInstance.AdvanceTime(time);

				clientInstance.SendPackets();
				clientInstance.ReceivePackets();
				
				next_frame_time += frame_interval;
				std::this_thread::sleep_until(next_frame_time);
			}//while

			std::cout << "Connected" << std::endl;

			while(1)
			{
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> total_elapsed = now - start_time;
				time = total_elapsed.count();

				clientInstance.AdvanceTime(time);

				clientInstance.ReceivePackets();

				if (!clientInstance.IsConnected())
        			break;
				
				jsonMessage* yeet = (jsonMessage*) clientInstance.CreateMessage(JSON_MESSAGE);

				uint8_t buffer[1024];
				yojimbo::WriteStream writeStream(buffer, sizeof(buffer));

				yeet->data["text"] = "Hello, world!";
				yeet->data["id"] = 42;

				// Serialize the message (write to stream)
				yeet->Serialize(writeStream);

				// Finish writing and get the number of bytes
				int bytesWritten = writeStream.GetBytesProcessed();
				printf("Wrote %d bytes\n", bytesWritten);

				clientInstance.SendMessage(0, yeet);

				clientInstance.SendPackets();

				next_frame_time += frame_interval;
				std::this_thread::sleep_until(next_frame_time);
			}//while

			clientInstance.Disconnect();

		} else {
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main