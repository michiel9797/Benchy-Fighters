//engine.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 12-04-2025

#include "engine.h"

//testing
#include <iostream>

playerstate::playerstate()
{
	health = 0;
	comboCount = 0;
	position[0] = 0;
	position[1] = 0;
	directionalForce[0] = 0;
	directionalForce[1] = 0;
	gravityScaling = 1;
	damageScaling = 0;
	action = actionList[0];
	frame = 0;
	inactionable = false;
	hasHit = false;
	hasBlocked = false;
	mirror = false;
}//playerstate

playerstate::playerstate(int player)
{
	health = 400;
	comboCount = 0;
	if(player == 1)
	{
		position[0] = 0;
		position[1] = 0;
	} else {
		position[0] = 200;
		position[1] = 0;
	}//else
	directionalForce[0] = 0;
	directionalForce[1] = 0;
	gravityScaling = 1;
	damageScaling = 1;
	action = actionList[0];
	frame = 0;
	inactionable = false;
	hasHit = false;
	hasBlocked = false;
	if(player == 1)
	{
		mirror = false;
	} else {
		mirror = true;
	}//else
}//playerstate

gamestate::gamestate()
{
	player[0] = playerstate(1);
	player[1] = playerstate(2);
	frame = 0;
	finished = false;
}//gamestate

gamestate engine::getGamestate(int requestedState)
{
	return statecache[requestedState];
}//getGamestate

int engine::initInput(int player, json data)
{
	for(int i = 0; i < data.size(); i++)
	{
		if(data[i]["Pressed"] != nullptr)
		{
			std::string buttonString = data[i]["Pressed"];
			char button = buttonString[1];
			std::string timeString = data[i]["Time"];
			float time = std::stof(timeString) * 1000;
			inputList[player-1].push(std::make_pair(button, time));
		}//if
	}//for
	if(inputList[player-1].size() <= 1)
	{
		std::cerr << "No input found" << std::endl;
		return -1;
	}
	return 0;
}//initInput

void engine::setCurrentPlayer(int player)
{
	currentPlayer = player;
}//setCurrentPlayer

int engine::manageInputs(float time, int player)
{
	//remove all the inputs from the vector which have passed
	//the buffers frame/time limit
	while(statecache.front().processingInput[player-1].back().second < 
		  ((statecache.front().frame * timePerFrame) - (framesInBuffer * timePerFrame)))
	{
		statecache.front().processingInput[player-1].erase(statecache.front().processingInput[player-1].end());
	}//while

	//add all the inputs from the input queue that have entered
	//the buffers frame/time limit
	while(inputList[player-1].front().second < statecache.front().frame * timePerFrame)
	{
		statecache.front().processingInput[player-1].push_back(inputList[player-1].front());
		inputList[player-1].pop();
	}//while
	if(inputList[player-1].empty())
		return 1;
	
	return 0;
}//manageInputs

void engine::setFirstFrame()
{
	statecache.reserve(7);
	gamestate firstState;
	statecache.insert(statecache.begin(), firstState);
}//setFirstFrame

gamestate engine::framegen()
{
	tickUpFrame();
	return statecache.front();
}//framegen

void engine::setPlayerInactionable(int player, bool set)
{
	statecache.front().player[player-1].inactionable = set;
}//setPlayerInactionable

void engine::tickUpFrame()
{
	statecache.front().frame += 1;
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//if the player isn't idling
		if(statecache.front().player[i-1].action.damage != -1)
		{
			statecache.front().player[i-1].frame += 1;
			//if the players action is over
			if(statecache.front().player[i-1].action.uptime < statecache.front().player[i-1].frame)
			{
				statecache.front().player[i-1].action = actionList[0];
				setPlayerInactionable(i, false);
				statecache.front().player[i-1].frame = 0;
			}//if
		}//if
	}//for
}//tickUpFrame

void engine::tickForceOnPlayer(int player)
{
	statecache.front().player[player-1].position[0] += statecache.front().player[player - 1].directionalForce[0];
	statecache.front().player[player-1].position[1] += statecache.front().player[player - 1].directionalForce[1];
}//changePosition

void engine::applyGravity(int player)
{
	statecache.front().player[player-1].directionalForce[1] -= gravity * statecache.front().player[player-1].gravityScaling;
}//applyGravity

void engine::resolveCollision(int player)
{
	//calculate values to index arrays
	//for player 1: targetPlayer = 0, otherPlayer = 1
	//for player 2: targetPlayer = 1, otherPlayer = 0
	int targetPlayer = player - 1;
	int otherPlayer = (player - 1) % 2;
	//check what distance between players we should calculate
	if(statecache.front().player[targetPlayer].mirror)
	{
		int playerDistance = abs(statecache.front().player[targetPlayer].position[0] - statecache.front().player[otherPlayer].position[0]);
		int overlap = actionList[0].hurtbox_dimensions[0][0] - playerDistance;
		statecache.front().player[targetPlayer].position[0] += overlap;
	} else {
		int targetPlayerRightEdge = statecache.front().player[targetPlayer].position[0] + actionList[0].hurtbox_dimensions[0][0];
		int overlap = abs(targetPlayerRightEdge - statecache.front().player[otherPlayer].position[0]);
		statecache.front().player[targetPlayer].position[0] -= overlap;
	}//else
}//resolveCollision

bool engine::detectCollision()
{
	std::vector<sf::FloatRect> firstCollisionBoxSet = createBox(actionList[0], actionList[0],
																statecache.front().player[0].position,
															    1, statecache.front().player[0].mirror,
															    false);
	sf::FloatRect firstCollisionBox = firstCollisionBoxSet.front();
	std::vector<sf::FloatRect> secondCollisionBoxSet = createBox(actionList[0], actionList[0],
																 statecache.front().player[1].position,
																 1, statecache.front().player[1].mirror,
																 false);
	sf::FloatRect secondCollisionBox = secondCollisionBoxSet.front();
	return firstCollisionBox.intersects(secondCollisionBox);
}//detectCollision

void engine::tickMovement()
{
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//remember if they were in the air before this tick
		bool playerInAir =  statecache.front().player[i-1].position[1] < 0;
		applyGravity(i);
		tickForceOnPlayer(i);
		//if the player ended up under the ground
		if(statecache.front().player[i-1].position[1] >= 0)
			statecache.front().player[i-1].position[1] = 0;
		//if the player went from air to ground this tick
		if(playerInAir && statecache.front().player[i-1].position[1] == 0)
		{
			setPlayerInactionable(i, false);
			statecache.front().player[i-1].action = actionList[0];
			statecache.front().player[i-1].comboCount = 0;
			statecache.front().player[i-1].damageScaling = 1;
			statecache.front().player[i-1].gravityScaling = 1;
			statecache.front().player[i-1].directionalForce[0] = 0;
			statecache.front().player[i-1].directionalForce[1] = 0;
		}
		//if the movement caused both players to collide
		if(detectCollision())
		{
			resolveCollision(i);
		}//if
	}//for
	//check which player is standing to the right and should be mirrored
	if(statecache.front().player[0].position[0] < statecache.front().player[0].position[0])
	{
		statecache.front().player[0].mirror = true;
		statecache.front().player[1].mirror = false;
	} else {
		statecache.front().player[0].mirror = false;
		statecache.front().player[1].mirror = true;
	}//else
}//tickMovement

void engine::checkActiveHitbox()
{
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//check if the frame their action is on has an active hitbox
		if(statecache.front().player[i-1].action.hitbox[0] <= statecache.front().player[i-1].frame &&
		   statecache.front().player[i-1].action.hitbox[1] >= statecache.front().player[i-1].frame)
		   statecache.front().player[i-1].hasHit = detectHit(i);
	}//for
}

void engine::applyHitEffects()
{
	//store if a player has been hit this frame
	bool wasHit[2] = {false, false};
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//calculate values to index arrays
		//for player 1: targetPlayer = 0, otherPlayer = 1
		//for player 2: targetPlayer = 1, otherPlayer = 0
		int targetPlayer = i - 1;
		int otherPlayer = (i - 1) % 2;
		//if the player has hit their move
		if(statecache.front().player[targetPlayer].hasHit == true)
		{
			//if the other player was blocking/holding back and didn't press an action button
			if(getMovementButton(otherPlayer) == 4 && getActionButton(otherPlayer) == -1)
			{
				float x_push = statecache.front().player[targetPlayer].action.launch_angle[0] *
							   statecache.front().player[targetPlayer].action.launch_force;
				//if the other player isn't mirrored, then to be pushed backwards
				//they need to be pushed to the left, so the push needs to be inverted
				if(!statecache.front().player[otherPlayer].mirror)
					x_push = -x_push;
				statecache.front().player[otherPlayer].position[0] += x_push;
				statecache.front().player[otherPlayer].hasBlocked = true;
				statecache.front().player[otherPlayer].inactionable = true;
			} else {
				float x_force = statecache.front().player[targetPlayer].action.launch_angle[0] *
							    statecache.front().player[targetPlayer].action.launch_force;
				float y_force = statecache.front().player[targetPlayer].action.launch_angle[1] *
								statecache.front().player[targetPlayer].action.launch_force;
				//same check as before
				if(!statecache.front().player[otherPlayer].mirror)
					x_force = -x_force;
				addForceToPlayer(otherPlayer, x_force, y_force);
				//deal at least 1 damage
				statecache.front().player[otherPlayer].health -= std::max((int)(statecache.front().player[targetPlayer].action.damage *
															statecache.front().player[otherPlayer].damageScaling), 1);
				statecache.front().player[otherPlayer].damageScaling *= damageScalingRate;
				statecache.front().player[otherPlayer].gravityScaling *= gravityScalingRate;
				statecache.front().player[otherPlayer].inactionable = true;
				wasHit[otherPlayer] = true;
			}//else	
		}//if
	}//for
	//check each player again, if they have hit and weren't hit
	//set them actionable again
	for(int i = 1; i <= 2; i++)
	{
		if(statecache.front().player[i-1].hasHit && !wasHit[i-1])
			statecache.front().player[i-1].inactionable = false;
	}//for
}//applyHitEffect

void engine::setNextActions()
{
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//if the player isn't inactionable, process their input
		if(statecache.front().player[i-1].inactionable = false)
		{
			int actionButton = getActionButton(i);
			int movementButton = getMovementButton(i);
			//if no action was pressed
			if(actionButton == -1)
			{
				//if no movement was pressed or only down was pressed
				if(movementButton == -1 || movementButton == 2)
				{
					continue;

				//if the movement is a jump
				} else if(movementButton == 8) 
				{
					statecache.front().player[i-1].directionalForce[1] += jumpForce;
					continue;

				//if the player wants to move left
				} else if(movementButton == 4)
				{
					statecache.front().player[i-1].position[0] -= movementAmount;

				//the movement must be to the right
				} else {
					statecache.front().player[i-1].position[0] += movementAmount;
				}
				//in case the player moved left or right, resolve any possible collisions
				resolveCollision(i);
				continue;
			
			//an action button has been pressed
			} else {
				std::array<int, 2> input = {movementButton, actionButton};
				actions chosenAction = getAction(input);
				//if the action isn't the idle action
				if(chosenAction.damage != -1)
				{
					statecache.front().player[i-1].inactionable = true;
				}//if
				statecache.front().player[i-1].action = chosenAction;
			}//else
		} else {
			//if the player blocked this turn
			if(statecache.front().player[i-1].hasBlocked)
			{
				statecache.front().player[i-1].inactionable = false;
				statecache.front().player[i-1].hasBlocked = false;
			}//if
		}//else
	}//for
}//setNextActions

void engine::addForceToPlayer(int player, float x_force, float y_force)
{
	statecache.front().player[player-1].directionalForce[0] += x_force;
	statecache.front().player[player-1].directionalForce[1] += y_force;
}//addForceToPlayer

void engine::changePlayerPosition(int player, float x, float y)
{
	statecache.front().player[player-1].position[0] += x;
	statecache.front().player[player-1].position[1] += y;
}//changePlayerPosition

int engine::getActionButton(int player)
{
	int i = 0;
	char input = '\0';
	while(input != 'w' && input != 'a' && input != 's' && input != 'd')
	{
		input = statecache[player-1].processingInput[player-1][i].first;
		i++;
	}//while
	if(input == '\0')
		return -1;
	return keymapping.at(input);
}//getAction

int engine::getMovementButton(int player)
{
	int i = statecache[player-1].processingInput[player-1].size();
	char input = '\0';
	while(input != 'u' && input != 'i' && input != 'j' && input != 'k' &&
		  i >= 0)
	{
		input = statecache[player-1].processingInput[player-1][i].first;
		i--;
	}//while
	if(input == '\0')
		return -1;
	return keymapping.at(input);
}//getMovement

std::array<int, 2> engine::getInput(int player)
{
	std::array<int, 2> input = {getMovementButton(player), getActionButton(player)};
	return input;
}//getInput

actions engine::getAction(std::array<int, 2> input)
{
	auto it = buttonMapping.find(input);
	//if the input does not match a mapping
	if(it == buttonMapping.end())
	{
		//see if there is a mapping for the same
		//button independent of the movement input
		input[0] = -1;
		it = buttonMapping.find(input);
		//if there isn't
		if(it == buttonMapping.end())
			return buttonMapping.at({-1, -1});
	}//if
	actions currentAction = it->second;
	return currentAction;
}//getInput

std::vector<sf::FloatRect> engine::createBox(actions action, actions idle, float playerLocation[2], int boxCount, 
						 				     bool mirror, bool grabHitbox)
{
	std::vector<sf::FloatRect> hitbox;
	if(grabHitbox)
	{
		if(!mirror)
		{
			for(int i = 0; i < boxCount; i++)
			{
				hitbox.push_back(sf::FloatRect(action.hitbox_origin[i][0] + playerLocation[0], 
											   action.hitbox_origin[i][1] + playerLocation[1],
											   action.hitbox_dimensions[i][0], action.hitbox_dimensions[i][1]));
			}//for
		} else {
			for(int i = 0; i < boxCount; i++)
			{
				//to get the mirrored hitbox, we need to mirror on the x axis. To do this,
				//we first need to grab the inverse of the x axis from the actions origin point,
				//subtract from this the dimensions of the hitbox since we need to take the top left
				//point instead of the top right. Then we need to subtract the width of the player
				//model from their location before adding it, to account for the fact that the
				//player origin point is at the top left of the player
				hitbox.push_back(sf::FloatRect((-action.hitbox_origin[i][0] - action.hitbox_dimensions[i][0]) 
											   + (playerLocation[0] + idle.hurtbox_dimensions[0][0]), 
											   action.hitbox_origin[i][1] + playerLocation[1],
											   action.hitbox_dimensions[i][0], action.hitbox_dimensions[i][1]));
			}//for
		}//else
	} else {
		if(!mirror)
		{
			for(int i = 0; i < boxCount; i++)
			{
				hitbox.push_back(sf::FloatRect(action.hurtbox_origin[i][0] + playerLocation[0], 
											   action.hurtbox_origin[i][1] + playerLocation[1],
											   action.hurtbox_dimensions[i][0], action.hurtbox_dimensions[i][1]));
			}//for
		} else {
			for(int i = 0; i < boxCount; i++)
			{
				//same mirroring method as before
				hitbox.push_back(sf::FloatRect((-action.hurtbox_origin[i][0] - action.hurtbox_dimensions[i][0]) 
											   + (playerLocation[0] + idle.hurtbox_dimensions[0][0]), 
											   action.hurtbox_origin[i][1] + playerLocation[1],
											   action.hurtbox_dimensions[i][0], action.hurtbox_dimensions[i][1]));
			}//for
		}//else
	}//else
	return hitbox;
}//createBox

bool engine::detectHit(int player)
{
	actions attacker;
	actions attackerIdle = actionList[0];
	float attackerLocation[2];
	bool mirrorAttacker;
	actions defender;
	actions defenderIdle = actionList[0];
	float defenderLocation[2];
	bool mirrorDefender;

	//calculate values to index arrays
	//for player 1: targetPlayer = 0, otherPlayer = 1
	//for player 2: targetPlayer = 1, otherPlayer = 0
	int targetPlayer = player - 1;
	int otherPlayer = (player - 1) % 2;

	attacker = statecache.front().player[targetPlayer].action;
	attackerLocation[0] = statecache.front().player[targetPlayer].position[0];
	attackerLocation[1] = statecache.front().player[targetPlayer].position[1];
	mirrorAttacker = statecache.front().player[targetPlayer].mirror;
	defender = statecache.front().player[otherPlayer].action;
	defenderLocation[0] = statecache.front().player[otherPlayer].position[0];
	defenderLocation[1] = statecache.front().player[otherPlayer].position[1];
	mirrorAttacker = statecache.front().player[otherPlayer].mirror;

	//since a move can use up to 3 hitboxes, and unused hitboxes
	//are demarked with a -1, this variable will check per loop if
	//a hitbox goes unused and should be left alone
	int hitboxUsed = 0;
	while(attacker.hitbox_origin[hitboxUsed][0] != -1 && hitboxUsed < 3)
	{
		hitboxUsed++;
	}//while

	std::vector<sf::FloatRect> hitboxes = createBox(attacker, attackerIdle, attackerLocation,
												    hitboxUsed, mirrorAttacker, true);

	int hurtboxUsed = 0;
	while(defender.hurtbox_origin[hurtboxUsed][0] != -1 && hurtboxUsed < 3)
	{
		hurtboxUsed++;
	}//while

	std::vector<sf::FloatRect> hurtboxes = createBox(defender, defenderIdle, defenderLocation,
													 hurtboxUsed, mirrorDefender, false);

	for(int j = 0; j < hurtboxUsed; j++)
	{
		for(int i = 0; i < hitboxUsed; i++)
		{
			if(hitboxes[i].intersects(hurtboxes[j]))
				return true;
		}//for
	}//for
	return false;
}//detectHit


//test functions
void engine::setAction(int player, int action)
{
	statecache.front().player[player-1].action = actionList[action];
	return;
}//setAction

bool engine::checkHit(int player)
{
	return detectHit(player);
}//checkHit

void engine::testGravity(int player, int action)
{
	statecache.front().player[player-1].directionalForce[0] += actionList[action].launch_angle[0] * actionList[action].launch_force;
	statecache.front().player[player-1].directionalForce[1] += actionList[action].launch_angle[1] * actionList[action].launch_force;
	applyGravity(1);
	tickForceOnPlayer(1);
	int frame = 0;
	while(statecache.front().player[player-1].position[1] != 0)
	{
		frame++;
		std::cout << "Frame: " << frame << std::endl;
		std::cout << "x: " << statecache.front().player[player-1].position[0] << std::endl;
		std::cout << "y: " << statecache.front().player[player-1].position[1] << std::endl;
		applyGravity(1);
		tickForceOnPlayer(1);
		if(statecache.front().player[player-1].position[1] < 0)
		{
			statecache.front().player[player-1].position[1] = 0;
			statecache.front().player[player-1].directionalForce[0] = 0;
			statecache.front().player[player-1].directionalForce[1] = 0;
		}//if
	}//while
	return;
}//testGravity