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

//for comparing the inputs in two json
bool compareInputJson(json data1, json data2)
{
	if(data1.size() != data2.size())
	{
		return false;
	}//if

	for(long unsigned int i = 0; i < data1.size(); i++)
	{
		if(data1[i]["Pressed"] != data2[i]["Pressed"])
		{
			return false;
		}//if
	}//for

	return true;
}//compareInputJson

//for setting the input times correctly in predictions
json setInputAhead(json data, int frameIncreaseAmount)
{
	for(long unsigned int i = 0; i < data.size(); i++)
	{
		std::string timeString = data[i]["Time"];
		float time = std::stof(timeString) * 1000;
		time += frameIncreaseAmount * timePerFrame;
		timeString = std::to_string(time);
		data[i]["Time"] = timeString;
	}//for
	return data;
}//setInputAhead

//for grabbing the inputs to send over to the other client
json extractInputForFrame(json data, int frame){
	//make a json object to copy inputs into
	json copy = json::array();
	//keep count of what line of the copy we're working on
	long unsigned int copyCount = 0;

	if(data.is_array())
	{
		for(long unsigned int i = 0; i < data.size(); i++)
		{
			if(data[i]["Pressed"] != nullptr)
			{
				std::string timeString = data[i]["Time"];
				float time = std::stof(timeString) * 1000;

				//if the input is earlier or later then we're looking for
				if((time < (frame - 1) * timePerFrame))
				{	//skip if earlier
					continue;
				}else if(time >= frame * timePerFrame)
				{	//stop if later
					break;
				}

				copy[copyCount] = data[i];
				copyCount++;
			}//if
		}//for
	}else{
		if(data["Pressed"] != nullptr)
		{
			std::string timeString = data["Time"];
			float time = std::stof(timeString) * 1000;

			//if the input is not earlier or later then we're looking for
			if((time >= (frame - 1) * timePerFrame) && (time < frame * timePerFrame))
				copy[copyCount] = data;
		}//if
	}//else

	//if we have no inputs to give
	if(copy[0]["Pressed"] == nullptr)
	{	//load in a dummy input
		json newCopy;
		newCopy["Pressed"] = "empty";
		return newCopy;
	}//if

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
void clientLoop(engine &gameEngine, yojimbo::Client &clientInstance, 
				double simTime, int currentPlayer, json currentPlayerInput)
{
	//timing logic
	auto frame_interval = std::chrono::milliseconds(timePerFrame);
	auto next_frame_time = std::chrono::high_resolution_clock::now();
	double sim_interval = timePerFrame / 1000.0;
	//the frame the game should be in, this is behind the client loop 
	//an amount of frames equal to the network delay
	int gameFrame = -(networkDelay);
	//a copy of the last input we've received, used for predictions
	json lastInputReceived;
	//the last time we've received an input
	int frameLastInputReceived = 0;
	//the amount of frames we need to roll back and resimulate
	int resimulate = 0;
	//if the match has started
	bool start = false;
	
	while(!gameEngine.getFinished())
	{
		simTime += sim_interval;
		clientInstance.AdvanceTime(simTime);

		clientInstance.ReceivePackets();

		if (!clientInstance.IsConnected())
			break;

		jsonMessage *message = (jsonMessage*)clientInstance.ReceiveMessage(0);

		//if we haven't had the start signal
		if(!start)
		{	//while we have messages
			while(message)	
			{	//if the message is the start signal
				if(message->data.is_object())
				{
					if(message->data["Start"] == "true")
					{
						start = true;
					}//if
				}//if
				message = (jsonMessage*)clientInstance.ReceiveMessage(0);
			}//while
		//if we've had the start signal
		}else{
			//if we get a message
			if(message)
			{	//if we're receiving messages we should have received earlier
				/*
				if(frameLastInputReceived < gameFrame)
				{	//set the max amount of frames to resimulate later
					resimulate = gameFrame - frameLastInputReceived;
					//for each message we've received
					while(message)
					{   //if the prediction was wrong
						if(!compareInputJson(message->data, lastInputReceived))
						{
							break;
						}//if
						//for every correct prediction in order, reduce the amount of frames
						//that need resimulation and throw away the corresponding message
						resimulate--;
						message = (jsonMessage*)clientInstance.ReceiveMessage(0);
					}//while
					//roll the game back by up to 7 frames
					gameEngine.rollback(std::min(resimulate, 7));
				}//if	
				*/
				//while we still have messages
				while(message)
				{
					if(message->data.is_array())
					{	//if the message isn't empty
						if(message->data[0]["Pressed"] != "empty")
						{	//process all message data for the opposite player
							if(currentPlayer == 1)
							{
								gameEngine.addInput(2, message->data);
							}else{
								gameEngine.addInput(1, message->data);
							}//else
						}//if
					}else{
						if(message->data["Pressed"] != "empty")
						{
							if(currentPlayer == 1)
							{
								gameEngine.addInput(2, message->data);
							}else{
								gameEngine.addInput(1, message->data);
							}//else
						}//if
					}//else
					lastInputReceived = message->data;
					message = (jsonMessage*)clientInstance.ReceiveMessage(0);
				}//while
				frameLastInputReceived = gameFrame + networkDelay;
			}//if

			//if we've waited out the network delay
			if(gameFrame >= 0)
			{
				/*
				//if we haven't received the inputs from the opponent we need for this frame
				if(frameLastInputReceived < gameFrame)
				{
					//create a copy of the last input and set it to this frame
					json prediction = setInputAhead(lastInputReceived, gameFrame - frameLastInputReceived);
					if(currentPlayer != 1)
					{
						gameEngine.addInput(2, prediction);
					}else{
						gameEngine.addInput(1, prediction);
					}//else
				}//if

				//resimulate if we need to
				for(int i = 0; i < resimulate; i++)
				{
					gameEngine.manageInputs(1);
					gameEngine.manageInputs(2);
					//do not print the resumulated frames
					gameEngine.framegen();
					gameEngine.prepStatecache();
				}//for*/

				//advance game state regularly
				gameEngine.manageInputs(1);
				gameEngine.manageInputs(2);
				
				gameEngine.printGamestate(gameEngine.framegen());
				gameEngine.prepStatecache();
			}//if*/

			//make a new message
			message = (jsonMessage*)clientInstance.CreateMessage(JSON_MESSAGE);
			//insert the required input data
			message->data = extractInputForFrame(currentPlayerInput, gameFrame + networkDelay);
			//send the message
			clientInstance.SendMessage(0, message);

			gameFrame++;
		}//else

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
			{	//generate and send the start message
				jsonMessage *message1 = (jsonMessage*)serverInstance.CreateMessage(0, JSON_MESSAGE);
				message1->data["Start"] = "true";
				jsonMessage *message2 = (jsonMessage*)serverInstance.CreateMessage(1, JSON_MESSAGE);
				message2->data["Start"] = "true";
				serverInstance.SendMessage(0, 0, message1);
				serverInstance.SendMessage(1, 0, message2);

				clientPair = true;
			}//if
		} else {
			//if a client has disconnected
			if(!(serverInstance.GetNumConnectedClients() == 2))
				return;

			//exchange messages between clients
			jsonMessage *message = (jsonMessage*)serverInstance.ReceiveMessage(0, 0);
			jsonMessage *copy;
			while(message)
			{
				copy = (jsonMessage*)serverInstance.CreateMessage(1, JSON_MESSAGE);
				copy->data = message->data;
				serverInstance.ReleaseMessage(0, message);

				serverInstance.SendMessage(1, 0, copy);
				message = (jsonMessage*)serverInstance.ReceiveMessage(0, 0);
			}//while
			message = (jsonMessage*)serverInstance.ReceiveMessage(1, 0);
			while(message)
			{
				copy = (jsonMessage*)serverInstance.CreateMessage(0, JSON_MESSAGE);
				copy->data = message->data;
				serverInstance.ReleaseMessage(1, message);

				serverInstance.SendMessage(0, 0, copy);
				message = (jsonMessage*)serverInstance.ReceiveMessage(1, 0);
			}//while
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
			}//if
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

			yojimbo::Address serverAddress(argv[5]);
			uint8_t privateKey[yojimbo::KeyBytes];
			memset(privateKey, 0, yojimbo::KeyBytes);

			auto frame_interval = std::chrono::milliseconds(timePerFrame);
			auto next_frame_time = std::chrono::high_resolution_clock::now();
			double sim_interval = timePerFrame / 1000.0;
			double simTime = 0.0;

			clientInstance.AdvanceTime(simTime);

			clientInstance.InsecureConnect(privateKey, currentPlayer, serverAddress);
			
			while(!clientInstance.IsConnected())
			{
				simTime += sim_interval;

				clientInstance.AdvanceTime(simTime);

				clientInstance.SendPackets();
				clientInstance.ReceivePackets();
				
				next_frame_time += frame_interval;
				std::this_thread::sleep_until(next_frame_time);
			}//while

			clientLoop(gameEngine, clientInstance, simTime, currentPlayer, data);

			clientInstance.Disconnect();

		} else {
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main