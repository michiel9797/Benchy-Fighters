//engine.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 09-04-2025

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
	damageScaling = 0;
	action = actionList[0];
	frame = 0;
	inactionable = false;
	hasHit = false;
}//playerstate

gamestate::gamestate()
{
	player[0] = playerstate(1);
	player[1] = playerstate(2);
	frame = 0;
}//gamestate

gamestate engine::getGamestate(int requestedState)
{
	return statecache[requestedState];
}//getGamestate

void engine::tickForceOnPlayer(int player)
{
	statecache[0].player[player-1].position[0] += statecache[0].player[player - 1].directionalForce[0];
	statecache[0].player[player-1].position[1] += statecache[0].player[player - 1].directionalForce[1];
}//changePosition

void engine::addForceToPlayer(int player, float x_force, float y_force)
{
	statecache[0].player[player-1].directionalForce[0] += x_force;
	statecache[0].player[player-1].directionalForce[1] += y_force;
}//addForceToPlayer

void engine::applyGravity(int player)
{
	statecache[0].player[player-1].directionalForce[1] -= gravity * statecache[0].player[player-1].gravityScaling;
}//applyGravity

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
				//to get the mirrored hitbox, we need to mirror on the x axis. To do this,
				//we first need to grab the inverse of the x axis from the actions origin point,
				//subtract from this the dimensions of the hitbox since we need to take the top left
				//point instead of the top right. Then we need to subtract the width of the player
				//model from their location before adding it, to account for the fact that the
				//player origin point is at the top left of the player
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
	actions defender;
	actions defenderIdle = actionList[0];
	float defenderLocation[2];
	if(player == 1)
	{
		attacker = statecache[0].player[0].action;
		attackerLocation[0] = statecache[0].player[0].position[0];
		attackerLocation[1] = statecache[0].player[0].position[1];
		defender = statecache[0].player[1].action;
		defenderLocation[0] = statecache[0].player[1].position[0];
		defenderLocation[1] = statecache[0].player[1].position[1];
	} else {
		attacker = statecache[0].player[1].action;
		attackerLocation[0] = statecache[0].player[1].position[0];
		attackerLocation[1] = statecache[0].player[1].position[1];
		defender = statecache[0].player[0].action;
		defenderLocation[0] = statecache[0].player[0].position[0];
		defenderLocation[1] = statecache[0].player[0].position[1];
	}//else

	std::cout << attacker.damage<< std::endl;
	std::cout << defender.damage << std::endl;

	//if the attacker is to the right of the defender instead of to their left, 
	//then the moves need to be flipped to face eachother
	bool mirrorAttacker = attackerLocation[0] > defenderLocation[0];
	bool mirrorDefender = !mirrorAttacker;

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
			std::cout << "hitbox " << i + 1 << std::endl;
			std::cout << "hurtbox " << j + 1 << std::endl;
			std::cout << "hitbox origin: " << hitboxes[i].left << ", " << hitboxes[i].top <<  std::endl;
			std::cout << "hitbox size: " << hitboxes[i].width << ", " << hitboxes[i].height << std::endl;
			std::cout << "hurtbox origin: " << hurtboxes[j].left << ", " << hurtboxes[j].top << std::endl;
			std::cout << "hurtbox size: " << hurtboxes[j].width << ", " << hurtboxes[j].height << std::endl;
			if(hitboxes[i].intersects(hurtboxes[j]))
				return true;
		}//for
	}//for
	return false;
}//detectHit


//test functions
void engine::setAction(int player, int action)
{
	statecache[0].player[player-1].action = actionList[action];
	return;
}//setAction

bool engine::checkHit(int player)
{
	return detectHit(player);
}//checkHit

void engine::testGravity(int player, int action)
{
	statecache[0].player[player-1].directionalForce[0] += actionList[action].launch_angle[0] * actionList[action].launch_force;
	statecache[0].player[player-1].directionalForce[1] += actionList[action].launch_angle[1] * actionList[action].launch_force;
	applyGravity(1);
	changePosition(1);
	int frame = 0;
	while(statecache[0].player[player-1].position[1] != 0)
	{
		frame++;
		std::cout << "Frame: " << frame << std::endl;
		std::cout << "x: " << statecache[0].player[player-1].position[0] << std::endl;
		std::cout << "y: " << statecache[0].player[player-1].position[1] << std::endl;
		applyGravity(1);
		changePosition(1);
		if(statecache[0].player[player-1].position[1] < 0)
		{
			statecache[0].player[player-1].position[1] = 0;
			statecache[0].player[player-1].directionalForce[0] = 0;
			statecache[0].player[player-1].directionalForce[1] = 0;
		}//if
	}//while
	return;
}//testGravity