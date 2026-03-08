//game_loop.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include <chrono>
#include <thread>
#include "game_loop.h"

using json = nlohmann::json;

//for comparing the inputs in two json
bool compareInputJson(std::vector<json> data1, std::vector<json> data2)
{
	if(data1.size() != data2.size())
		return false;

	for(long unsigned int i = 0; i < data1.size(); i++)
	{
		if(data1[i]["Pressed"] != data2[i]["Pressed"])
			return false;
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
json extractInputForFrame(json data, int extractFrame)
{
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
				}//else if

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

//the main loop for a local run
void localLoop(engine &gameEngine)
{
	auto frame_interval = std::chrono::milliseconds(int(timePerFrame)); 
    auto current = std::chrono::high_resolution_clock::now();
    while (!gameEngine.getFinished()) {
		gameEngine.prepStatecache();
		gameEngine.manageInputs(1);
		gameEngine.manageInputs(2);
        gameEngine.printGamestate(gameEngine.framegen());
        current += frame_interval;
		std::this_thread::sleep_until(current);
    }//while
}//localLoop

//the main loop for a client in the simulation run
void clientLoop(engine &gameEngine, clientLayer *client, json currentPlayerInput)
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
		if(!client->startOfLoop(simInterval))
			return;

		std::vector<json> message;

		//if we haven't had the start signal
		if(!start)
		{	//while we have messages
			while(client->receiveMessage(message))	
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
			if(client->hasMessageToReceive())
			{	//if we're receiving messages we should have received earlier
				if(frameLastInputReceived < gameFrame)
				{	//set the max amount of frames to resimulate later
					resimulate = gameFrame - frameLastInputReceived + 1;

					//create temporary storage for the last message data we received
					std::vector<json> tempInput;

					//for each message we've received, if we would still have to resimulate
					while(resimulate > 0 && client->receiveMessage(message))
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
					if((!hasMessage && !client->hasMessageToReceive()) && resimulate > 0)
						resimulate = 0;

					//roll the game back by up to 8 frames
					resimulate = std::min(resimulate, 8);
					gameEngine.rollback(resimulate);
				}//if	

				//while we still have messages
				while(hasMessage || client->receiveMessage(message))
				{	//if the message isn't empty
					if(!message[0]["Pressed"].is_null() && message[0]["Pressed"] != "empty")
					{	//process all message data for the opposite player
						if(gameEngine.getCurrentPlayer() == 1)
						{
							gameEngine.addInput(2, message);
						}else
							gameEngine.addInput(1, message);
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
							}else
								gameEngine.addInput(1, prediction);
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
			client->sendMessage(messageData);

			gameFrame++;
		}//else

		client->endOfLoop();

		nextFrameTime += frameInterval;
		std::this_thread::sleep_until(nextFrameTime);
	}//while
}//clientLoop

//the main loop for a server in the simulation run
void serverLoop(serverLayer *server)
{
	auto frameInterval = std::chrono::milliseconds(int(timePerFrame));
	auto nextFrameTime = std::chrono::high_resolution_clock::now();
	double simInterval = timePerFrame / 1000.0;
	//has the start signal been sent
	bool start = false;

	while (true)
	{
		if(!server->startOfLoop(simInterval))
			return;

		//if the start signal hasn't been sent yet
		if(!start)
		{	
			start = server->startMatch();
		} else
			server->exchangeMessages();

		if(!server->endOfLoop())
			return;

		nextFrameTime += frameInterval;
		std::this_thread::sleep_until(nextFrameTime);
	}//while
}//serverLoop