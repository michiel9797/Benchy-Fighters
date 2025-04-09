//engine.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 08-04-2025

#include "engine.h"

playerstate::playerstate()
{
	health = 0;
	comboCount = 0;
	position[0] = 0;
	position[1] = 0;
	direction[0] = 0;
	direction[1] = 0;
	force = 0;
	gravity = 1;
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
		position[0] = 150;
		position[1] = 0;
	}//else
	direction[0] = 0;
	direction[1] = 0;
	force = 0;
	gravity = 1;
	damageScaling = 0;
	action = actionList[0];
	frame = 0;
	inactionable = false;
	hasHit = false;
}//playerstate

gamestate::gamestate(int device)
{
	if(device == 1)
	{
		player[0] = playerstate(1);
		player[1] = playerstate(2);
	} else {
		player[0] = playerstate(2);
		player[1] = playerstate(1);
	}//else
	frame = 0;
}//gamestate

gamestate engine::getGamestate(int requestedState)
{
	return statecache[requestedState];
}//getGamestate

sf::FloatRect* engine::createBox(actions action, actions idle, float playerLocation[2], int boxCount, 
						 bool mirror)
{
	sf::FloatRect hitbox[boxCount];
	if(!mirror)
	{
		for(int i = 0; i < boxCount; i++)
		{
			hitbox[i] = sf::FloatRect(action.hitbox_origin[i][0] + playerLocation[0], 
								  	  action.hitbox_origin[i][1] + playerLocation[1],
								  	  action.hitbox_dimensions[i][0], action.hitbox_dimensions[i][1]);
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
			hitbox[i] = sf::FloatRect((-action.hitbox_origin[i][0] - action.hitbox_dimensions[i][0]) 
									  + (playerLocation[0] - idle.hitbox_dimensions[0][0]), 
								  	  action.hitbox_origin[i][1] + playerLocation[1],
								  	  action.hitbox_dimensions[i][0], action.hitbox_dimensions[i][1]);
		}//for
	}//else
	sf::FloatRect* pointer = &hitbox[0];
	return pointer;
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
		attacker = statecache[0].player[1].action;
		attackerLocation[0] = statecache[0].player[1].position[0];
		attackerLocation[1] = statecache[0].player[1].position[1];
		defender = statecache[0].player[2].action;
		defenderLocation[0] = statecache[0].player[2].position[0];
		defenderLocation[1] = statecache[0].player[2].position[1];
	} else {
		attacker = statecache[0].player[2].action;
		attackerLocation[0] = statecache[0].player[2].position[0];
		attackerLocation[1] = statecache[0].player[2].position[1];
		defender = statecache[0].player[1].action;
		defenderLocation[0] = statecache[0].player[1].position[0];
		defenderLocation[1] = statecache[0].player[1].position[1];
	}//else

	//if the attacker is to the right of the defender instead of to their left, 
	//then the moves need to be flipped to face eachother
	bool mirror = attackerLocation[0] > defenderLocation[0];

	//since a move can use up to 3 hitboxes, and unused hitboxes
	//are demarked with a -1, this variable will check per loop if
	//a hitbox goes unused and should be left alone
	int hitboxUsed = 0;
	while(attacker.hitbox_origin[hitboxUsed][0] != -1)
	{
		hitboxUsed++;
	}//while

	sf::FloatRect* hitboxes = createBox(attacker, attackerIdle, attackerLocation,
												   hitboxUsed, mirror);

	int hurtboxUsed = 0;
	while(defender.hitbox_origin[hitboxUsed][0] != -1)
	{
		hurtboxUsed++;
	}//while

	sf::FloatRect* hurtboxes = createBox(defender, defenderIdle, defenderLocation,
													 hurtboxUsed, mirror);

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