//engine.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef EngineH
#define EngineH

#include <deque>
#include <vector>
#include <utility>
#include <cmath>
#include <map>
#include <SFML/Graphics/Rect.hpp>

#include "nlohmann-json/json.hpp"
#include "action_list.h"

using json = nlohmann::json;

const short jumpForce = 30;
const short jumpSideForce = 15;
const short movementAmount = 4;
const float gravity = 1.5;
const float gravityScalingRate = 1.2;
const float damageScalingRate = 0.8;
const float framerate = 60;
//frametime is stored in miliseconds
const float timePerFrame = 1000 / framerate;
//the amount of frames to keep inputs in the processing buffer
const short framesInBuffer = ceil(framerate * 0.25);
//the full amount of frames that will be generated if
//the match doesn't end prematurely
const short maxFrames = framerate * 99;

const std::map<char, short> keymapping
{
	//up = 8
	{'w', 8},
	//left = 4
	{'a', 4},
	//down = 2
	{'s', 2},
	//right = 6
	{'d', 6},
	//u = punch = 0
	{'u', 0},
	//i = kick = 1
	{'i', 1},
	//j = slash = 2
	{'j', 2},
	//k = heavy slash = 3
	{'k', 3}
};//keymapping

//information needed to define the state a player is in
struct playerstate
{
	playerstate();
	//player 1 stands left, player 2 stands right
	playerstate(int player);
	//the players current health
	short health;
	//how many times a player has been hit before touching the ground
	short comboCount;
	//the position of a player
	float position[2];
	//the direction a player is going in and the force by which they are
	//moving in said direction
	float directionalForce[2];
	//where a player is walking in if applicable
	short walk;
	//the strenght of gravity affecting the player
	float gravityScaling;
	//by how much to reduce incoming damage
	float damageScaling;
	//the current action being performed
	actions action;
	//what frame of the action the player is in
	short frame;
	//if the player is currently inactionable
	bool inactionable;
	//if the players current action has hit
	bool hasHit;
	//if the player has sucessfully blocked this frame
	bool hasBlocked;
	//if the players actions and inputs should be mirrored horizontally
	bool mirror;
};//playerstate

//information needed to define the state the game is in
struct gamestate
{   //at round start the player from device 1 stands left, from device 2 stands right
	gamestate();
	//the state of both players
	playerstate player[2];
	//the current frame the match is on 
	short frame;
	//a deque of inputs to process for both players sorted by ascending frame number
	std::deque<std::pair<char, int>> processingInput[2];
	//if the current frame has finished calculations
	bool finished;
};//gamestate


class engine
{
	public:
		//return the requested stored gamestate
		const gamestate getGamestate(const short requestedState);
		//set the frontmost gamestate to the given gamestate
		void setGamestate(const gamestate state);
		//check if the game has finished
		const bool getFinished();
		//print all gamestate information to commandline
		const void printGamestate(const gamestate state);
		//load the given input data into the chosen players
		//input list
		short addInput(const short player, json data);
		//set who is playing on this device. -1 if we are running
		//in EMULATE mode
		void setCurrentPlayer(const short player);
		//get who is playing on this device
		const short getCurrentPlayer();
		//move inputs between buffers depending on the current frame
		short manageInputs(const short player);
		//initialize the first frame
		void setFirstFrame();
		//if we are at the 7 frame limit, move all elements back to allow the
		//first frame to be overwritten
		void prepStatecache();
		//generate and return the next frame given the current first frame
		//and the input lists
		gamestate framegen();
		//roll the gamestate back by the amount of frames given
		void rollback(const short rollbackFrames);

		//test functions
		void setAction(const short player, const short action);
		const bool checkHit(const short player);
		void testGravity(const short player, const short action);
		void printInputBuffer(const short player);

	private:
		//when rolling back, this pushes the current players
		//inputs for that frame back into the input buffer so it
		//can be re-evaluated again
		void pushInputBack();
		//set a players inactionable value to the given bool
		//1 = player 1, 2 = player 2
		void setPlayerInactionable(const short player, const bool set);
		//tick up the frame in the gamestate and on any non-idle
		//actions being performed. If an action crosses over its
		//uptime limit, replace it with idle and set the player
		//actionable
		void tickUpFrame();
		//move a player according to their directional forces
		void tickForceOnPlayer(const short player);
		//calculate gravity into a players directional force
		void applyGravity(const short player);
		//resolve collisions between players, pushing the given player out
		void resolveCollision(const short player);
		//check if both players are colliding
		const bool detectCollision();
		//tick all movement caused by forces on players, if a player hits
		//the ground this way reset all of their combo values and set them
		//actionable again. check which player should be mirrored and move
		//players out of eachother if movement caused them to take up the
		//same space
		void tickMovement();
		//check if any action currently has an active hitbox, if so check
		//if that move has hit
		void checkActiveHitbox();
		//apply any effects that occur when a player has landed a hit
		void applyHitEffects();
		//set the next action or apply movement to the players
		void setNextActions();
		//check if the game is over or not, this happens when a players health reaches 0
		//or if the framecap is reached
		bool gameOver();
		//add the given directional forces to a players directional force
		void addForceToPlayer(const short player, const float x_force, const float y_force);
		//change the players position directly
		void changePlayerPosition(const short player, const float x, const float y);
		//extract the action value from the input buffer
		//returns -1 if no action was found
		const short getActionButton(const short player);
		//extract the movement direction from the input buffer
		//returns -1 if no movement direction was found
		const short getMovementButton(const short player);
		//read the current input from the input buffer
		const std::array<short, 2> getInput(const short player);
		//get the action from the given input
		const actions getAction(std::array<short, 2> input);
		//return the hitboxes or hurtboxes from the given action at the
		//asked location. Returns up to 3 hitboxes or hurtboxes, any unused
		//boxes will be uninitialized. Idle needs to contain the player action
		//that stores the collision box of the player, as the top left of the
		//collision box is the origin point of actions and its width may be
		//needed. When mirror is set true, the created box will be mirrored
		//in location horizontally. If hitbox is true, a hitbox will be calcuclated.
		//otherwise a hurtbox will be calculated
		const std::vector<sf::FloatRect> createBox(const actions action, const actions idle, const float playerLocation[2],
								 			 const short boxCount, const bool mirror, const bool grabHitbox);
		//check if a player is hit by an action
		//the given players hitboxes will be checked against
		//the other players hurtboxes
		const bool detectHit(const short player);
		//gamestates are ordered from new to old, so:
		//0: most recent gamestate
		//1: gamestate after calculating 1 frame ago
		//...
		//6: gamestate after calculating 6 frames ago
		std::vector<gamestate> statecache;
		//a deque of inputs for both players 
		std::deque<std::pair<char, short>> inputList[2];
		//the player that is playing on this device, 1 or 2
		short currentPlayer;
};//engine

#endif