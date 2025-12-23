//engine.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 16-04-2025

#ifndef EngineH
#define EngineH

#include <stack>
#include <queue>
#include <deque>
#include <vector>
#include <utility>
#include <cmath>
#include <map>
#include <SFML/Graphics/Rect.hpp>
#include "nlohmann-json/json.hpp"

#include "action_list.h"

using json = nlohmann::json;

const int jumpForce = 30;
const int jumpSideForce = 15;
const int movementAmount = 4;
const float gravity = 1.5;
const float gravityScalingRate = 1.2;
const float damageScalingRate = 0.8;
const float framerate = 60;
//frametime is stored in miliseconds
const float timePerFrame = 1000 / framerate;
//the amount of frames to keep inputs in the processing buffer
const int framesInBuffer = ceil(framerate * 0.25);
//the full amount of frames that will be generated if
//the match doesn't end prematurely
const int maxFrames = framerate * 99;

const std::map<char, int> keymapping
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
	int health;
	//how many times a player has been hit before touching the ground
	int comboCount;
	//the position of a player
	float position[2];
	//the direction a player is going in and the force by which they are
	//moving in said direction
	float directionalForce[2];
	//where a player is walking in if applicable
	int walk;
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
	//if the player has sucessfully blocked this frame
	bool hasBlocked;
	//if the players actions and inputs should be mirrored horizontally
	bool mirror;
};//playerstate

//information needed to define the state the game is in
struct gamestate
{
	//at round start the player from device 1 stands left, from device 2 stands right
	gamestate();
	//the state of both players
	playerstate player[2];
	//the current frame the match is on 
	int frame;
	//a deque of inputs to process for both players sorted by ascending frame number
	std::deque<std::pair<char, int>> processingInput[2];
	//if the current frame has finished calculations
	bool finished;
};//gamestate


class engine
{
	public:
		//return the requested stored gamestate
		gamestate getGamestate(int requestedState);
		//set the frontmost gamestate to the given gamestate
		void setGamestate(gamestate state);
		//check if the game has finished
		bool getFinished();
		//print all gamestate information to commandline
		void printGamestate(gamestate state);
		//load the given input data into the chosen players
		//input list
		int addInput(int player, json data);
		//set who is playing on this device. -1 if we are running
		//in EMULATE mode
		void setCurrentPlayer(int player);
		//move inputs between buffers depending on the current frame
		int manageInputs(int player);
		//initialize the first frame
		void setFirstFrame();
		//if we are at the 7 frame limit, move all elements back to allow the
		//first frame to be overwritten
		void prepStatecache();
		//generate and return the next frame given the current first frame
		//and the input lists
		gamestate framegen();
		//roll the gamestate back by the amount of frames given
		void rollback(int rollbackFrames);

		//test functions
		void setAction(int player, int action);
		bool checkHit(int player);
		void testGravity(int player, int action);
		void printInputBuffer(int player);

	private:
		//when rolling back, this pushes the current players
		//inputs for that frame back into the input buffer so it
		//can be re-evaluated again
		void pushInputBack();
		//set a players inactionable value to the given bool
		//1 = player 1, 2 = player 2
		void setPlayerInactionable(int player, bool set);
		//tick up the frame in the gamestate and on any non-idle
		//actions being performed. If an action crosses over its
		//uptime limit, replace it with idle and set the player
		//actionable
		void tickUpFrame();
		//move a player according to their directional forces
		void tickForceOnPlayer(int player);
		//calculate gravity into a players directional force
		void applyGravity(int player);
		//resolve collisions between players, pushing the given player out
		void resolveCollision(int player);
		//check if both players are colliding
		bool detectCollision();
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
		void addForceToPlayer(int player, float x_force, float y_force);
		//change the players position directly
		void changePlayerPosition(int player, float x, float y);
		//extract the action value from the input buffer
		//returns -1 if no action was found
		int getActionButton(int player);
		//extract the movement direction from the input buffer
		//returns -1 if no movement direction was found
		int getMovementButton(int player);
		//read the current input from the input buffer
		std::array<int, 2> getInput(int player);
		//get the action from the given input
		actions getAction(std::array<int, 2> input);
		//return the hitboxes or hurtboxes from the given action at the
		//asked location. Returns up to 3 hitboxes or hurtboxes, any unused
		//boxes will be uninitialized. Idle needs to contain the player action
		//that stores the collision box of the player, as the top left of the
		//collision box is the origin point of actions and its width may be
		//needed. When mirror is set true, the created box will be mirrored
		//in location horizontally. If hitbox is true, a hitbox will be calcuclated.
		//otherwise a hurtbox will be calculated
		std::vector<sf::FloatRect> createBox(actions action, actions idle, float playerLocation[2],
								 			 int boxCount, bool mirror, bool grabHitbox);
		//check if a player is hit by an action
		//the given players hitboxes will be checked against
		//the other players hurtboxes
		bool detectHit(int player);
		//gamestates are ordered from new to old, so:
		//0: most recent gamestate
		//1: gamestate after calculating 1 frame ago
		//...
		//6: gamestate after calculating 6 frames ago
		std::vector<gamestate> statecache;
		//a deque of inputs for both players 
		std::deque<std::pair<char, int>> inputList[2];
		//the player that is playing on this device, 1 or 2
		int currentPlayer;
};//engine

#endif