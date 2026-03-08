//game_loop.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef GameLoopH
#define GameLoopH

#include "engine.h"
#include "yojimbo_layer.h"

using json = nlohmann::json;

//compare two json input data files
//returns true if all the data fields the engine uses are equal,
//returns false if they are not
//this means that fields that the engine doesn't use can be added
//without this check failing, as long as the size of the data files
//is still equal
bool compareInputJson(std::vector<json> data1, std::vector<json> data2);

//sets the frame in which an input occured ahead by a given amount
//returns a modified json file with the frame of each input set ahead
json setInputAhead(json data, int frameIncreaseAmount);

//extracts all the inputs for the specified frame and returns them
//returns a json file with all the messages from the specified frame
json extractInputForFrame(json data, int extractFrame);

//the main loop for a local run
//prints all generated game states to the terminal
void localLoop(engine &gameEngine);

//the main loop for a client in a multiplayer run
//prints all generated game states to the terminal
//does not get used to connect to a server, only runs the game
void clientLoop(engine &gameEngine, clientLayer *client, json currentPlayerInput);

//the main loop for a server in a multiplayers run
void serverLoop(serverLayer *server);

#endif