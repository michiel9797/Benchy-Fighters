//engine.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 05-04-2025

#ifndef EngineH
#define EngineH

#include <tuple>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include "action_list.h"
using json = nlohmann::json;


//information needed to define the state a player is in
struct playerstate
{
	//the players current health
	int health;
	//how many times a player has been hit before touching the ground
	int combo_count;
	//the position of a player
	float position;
	//the direction a player is going in
	std::tuple <float, float> direction;
	//the force with which a player is going into a direction
	float force;
	//the strenght of gravity affecting the player
	float gravity;
	//by how much to reduce incoming damage
	float damageScaling;
	//the current action being performed
	actions action;
	//what frame of the action the player is in
	int frame;
	//if the player is currently inactionable
	bool inactionable;
};//playerstate

//information needed to define the state the game is in
struct gamestate
{
	//the state of both players
	playerstate player[2];
	
};//gamestate


class engine
{
	public:
		//return the requested stored gamestate
		gamestate getGamestate(int requestedState);

	private:
		//gamestates are ordered from new to old, so:
		//0: most recent gamestate
		//1: gamestate 1 frame ago
		//...
		//6: gamestate 6 frames ago
		gamestate statecache[7];
		//the full list of inputs
		json inputList;
		//the full list of available player actions
		actions* actionList;
		//the amount of actions in the action list
		int actionCount;


};//engine

#endif