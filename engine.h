//engine.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 09-04-2025

#ifndef EngineH
#define EngineH

#include <SFML/Graphics/Rect.hpp>
#include "nlohmann-json/json.hpp"
#include "action_list.h"
using json = nlohmann::json;

const int gravity = 3;

//information needed to define the state a player is in
struct playerstate
{
	playerstate();
	//player 1 stands left, player 2 stands right
	playerstate(int player);
	//the players current health
	int health;
	//how many times a player has been hit before touching the ground
	int comboCount;
	//the position of a player
	float position[2];
	//the direction a player is going in and the force by which they are
	//moving in said direction
	float directionalForce[2];
	//the strenght of gravity affecting the player
	float gravityScaling;
	//by how much to reduce incoming damage
	float damageScaling;
	//the current action being performed
	actions action;
	//what frame of the action the player is in
	int frame;
	//if the player is currently inactionable
	bool inactionable;
	//if the players current action has hit
	bool hasHit;
};//playerstate

//information needed to define the state the game is in
struct gamestate
{
	//the player from device 1 stands left, from device 2 stands right
	gamestate();
	//the state of both players
	playerstate player[2];
	//the current frame the match is on 
	int frame;
};//gamestate


class engine
{
	public:
		//return the requested stored gamestate
		gamestate getGamestate(int requestedState);

		//test functions
		void setAction(int player, int action);
		bool checkHit(int player);
		void testGravity(int player, int action);

	private:
		//move a player according to their directional forces
		//1 = player 1, 2 = player 2
		void tickForceOnPlayer(int player);
		//add the given directional forces to a players directional force
		void addForceToPlayer(int player, float x_force, float y_force);
		//calculate gravity into a players directional force
		void applyGravity(int player);
		//return the hitboxes or hurtboxes from the given action at the
		//asked location. Returns up to 3 hitboxes or hurtboxes, any unused
		//boxes will be uninitialized. Idle needs to contain the player action
		//that stores the collision box of the player, as the top left of the
		//collision box is the origin point of actions and its width may be
		//needed. When mirror is set true, the created box will be mirrored
		//in location horizontally. if hitbox is true, a hitbox will be calcuclated.
		//otherwise a hurtbox will be calculated
		std::vector<sf::FloatRect> createBox(actions action, actions idle, float playerLocation[2],
								 			 int boxCount, bool mirror, bool hitbox);
		//check if a player is hit by an action
		//the given players hitboxes will be checked against
		//the other players hurtboxes
		//1 = player 1, 2 = player 2
		bool detectHit(int player);
		//gamestates are ordered from new to old, so:
		//0: most recent gamestate
		//1: gamestate 1 frame ago
		//...
		//6: gamestate 6 frames ago
		gamestate statecache[7];
		//the full list of inputs
		json inputList;
		//the player that is playing on this device
		int currentPlayer;


};//engine

#endif