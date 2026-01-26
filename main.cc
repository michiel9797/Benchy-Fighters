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

//for comparing the inputs in two json
bool compareInputJson(std::vector<json> data1, std::vector<json> data2)
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
	if(data.is_array())
	{
		for(long unsigned int i = 0; i < data.size(); i++)
		{
			std::string frameString = data[i]["Frame"];
			int frame = std::stoi(frameString) + frameIncreaseAmount;
			frameString = std::to_string(frame);
			data[i]["Frame"] = frameString;
		}//for
	}else{
		std::string frameString = data["Frame"];
		int frame = std::stoi(frameString) + frameIncreaseAmount;
		frameString = std::to_string(frame);
		data["Frame"] = frameString;
	}//else
	return data;
}//setInputAhead

//for grabbing the inputs to send over to the other client
json extractInputForFrame(json data, int extractFrame){
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
				std::string frameString = data[i]["Frame"];
				int frame = std::stoi(frameString);

				//if the input is earlier then we're looking for
				if((frame < extractFrame))
				{	//skip if earlier
					continue;
				}else if(frame > extractFrame)
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
			std::string frameString = data["Frame"];
			int frame = std::stoi(frameString);

			//if the input is in the correct frame
			if(frame == extractFrame)
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
	auto frame_interval = std::chrono::milliseconds(int(timePerFrame)); 
    auto current = std::chrono::high_resolution_clock::now();
    while (!gameEngine.getFinished()) {
		gameEngine.prepStatecache();
		gameEngine.manageInputs(1);
		gameEngine.manageInputs(2);
        gameEngine.printGamestate(gameEngine.framegen());
		//gameEngine.printInputBuffer(1);   //REMOVE!!!!!!!!!!
        current += frame_interval;
		std::this_thread::sleep_until(current);
    }//while
}//localLoop

//the main loop for a client in the simulation run
void clientLoop(engine &gameEngine, clientLayer &client, json currentPlayerInput)
{
	//timing logic
	auto frameInterval = std::chrono::milliseconds(int(timePerFrame));
	auto nextFrameTime = std::chrono::high_resolution_clock::now();
	double simInterval = timePerFrame / 1000.0;
	//the frame the game should be in, this is behind the client loop 
	//an amount of frames equal to the network delay
	int gameFrame = -(networkDelay);
	//a copy of the last input we've received, used for predictions
	std::vector<json> lastInputReceived;
	//the last time we've received an input
	int frameLastInputReceived = 0;
	//the amount of frames we need to roll back and resimulate
	int resimulate = 0;
	//if the match has started
	bool start = false;
	//if we currently have a non-processed message loaded in
	bool hasMessage = false;
	
	while(!gameEngine.getFinished())
	{
		if(!client.startOfLoop(simInterval))
			return;

		std::vector<json> message;

		//if we haven't had the start signal
		if(!start)
		{	//while we have messages
			while(client.receiveMessage(message))	
			{	//if the message is the start signal
				if(!message[0]["Start"].is_null() && message[0]["Start"] == "true")
				{
					start = true;
					//set up a stub for predictions if needs be
					lastInputReceived = message;
				}//if
			}//while
		//if we've had the start signal
		} else {
			//if we get a message
			if(client.hasMessageToReceive())
			{	//if we're receiving messages we should have received earlier
				if(frameLastInputReceived < gameFrame)
				{	//set the max amount of frames to resimulate later
					resimulate = gameFrame - frameLastInputReceived + 1;

					//create temporary storage for the last message data we received
					std::vector<json> tempInput;

					//for each message we've received, if we would still have to resimulate
					while(resimulate > 0 && client.receiveMessage(message))
					{
						//if the prediction was wrong
						if(!compareInputJson(message, lastInputReceived))
						{
							hasMessage = true;
							break;
						}//if
						tempInput = message;
						//for every correct prediction in order, reduce the amount of frames
						//that need resimulation and throw away the corresponding message
						resimulate--;
						frameLastInputReceived++;
					}//while
					//set the last input received correct again
					lastInputReceived = tempInput;
					//if we would still have to resimulate but have no messages
					//to process, postpone resimulation until we have the required
					//messages
					if((!hasMessage && !client.hasMessageToReceive()) && resimulate > 0)
					{
						resimulate = 0;
					}//if
					//roll the game back by up to 8 frames
					resimulate = std::min(resimulate, 8);
					gameEngine.rollback(resimulate);
				}//if	

				//while we still have messages
				while(hasMessage || client.receiveMessage(message))
				{	//if the message isn't empty
					if(!message[0]["Pressed"].is_null() && message[0]["Pressed"] != "empty")
					{	//process all message data for the opposite player
						if(gameEngine.getCurrentPlayer() == 1)
						{
							gameEngine.addInput(2, message);
						}else{
							gameEngine.addInput(1, message);
						}//else
					}//if
					lastInputReceived = message;
					frameLastInputReceived++;
					hasMessage = false;
				}//while
			}//if

			//if we've waited out the network delay
			if(gameFrame >= 0)
			{
				//if we haven't received the inputs from the opponent we need for this frame
				if(frameLastInputReceived < gameFrame)
				{
					int sendAhead = gameFrame - frameLastInputReceived;
					for(unsigned long i = 0; i < lastInputReceived.size(); i++)
					{
						if(lastInputReceived[i]["Pressed"] != "empty")
						{
							json prediction = setInputAhead(lastInputReceived, sendAhead);
							if(gameEngine.getCurrentPlayer() == 1)
							{
								gameEngine.addInput(2, prediction);
							}else{
								gameEngine.addInput(1, prediction);
							}//else
						}//if
					}//for
				}//if

				//resimulate if we need to
				for(int i = 0; i < resimulate; i++)
				{
					gameEngine.prepStatecache();
					gameEngine.manageInputs(1);
					gameEngine.manageInputs(2);
					//do not print the resimulated frames
					gameEngine.framegen();
				}//for

				resimulate = 0;

				//advance game state regularly
				gameEngine.prepStatecache();
				gameEngine.manageInputs(1);
				gameEngine.manageInputs(2);
				gameEngine.printGamestate(gameEngine.framegen());
			}//if
			
			json messageData = extractInputForFrame(currentPlayerInput, gameFrame + networkDelay);

			//send the message
			client.sendMessage(messageData);

			gameFrame++;
		}//else

		client.endOfLoop();

		nextFrameTime += frameInterval;
		std::this_thread::sleep_until(nextFrameTime);
	}//while
}//clientLoop

//the main loop for a server in the simulation run
void serverLoop(serverLayer &server)
{
	auto frameInterval = std::chrono::milliseconds(int(timePerFrame));
	auto nextFrameTime = std::chrono::high_resolution_clock::now();
	double simInterval = timePerFrame / 1000.0;
	//has the start signal been sent
	bool start = false;

	while (true)
	{
		if(!server.startOfLoop(simInterval))
			return;

		//if the start signal hasn't been sent yet
		if(!start)
		{	
			start = server.startMatch();
		} else {
			server.exchangeMessages();
		}//else

		if(!server.endOfLoop())
			return;

		nextFrameTime += frameInterval;
		std::this_thread::sleep_until(nextFrameTime);
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
		yojimboServer server = yojimboServer(argv[2]);

		std::cout << "SERVER START" << std::endl;

		serverLoop(server);

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
		
			yojimboClient client = yojimboClient(currentPlayer, argv[4]);

			if(client.startConnection(argv[5]))
				clientLoop(gameEngine, client, data);

			client.endConnection();

		} else {
			std::cerr << "Incorrect program call, call \"BenchyFighters --help\" for instructions" << std::endl;
			return -1;
		}//else
	}//else
	return 0;
}//main