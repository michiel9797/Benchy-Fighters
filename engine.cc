//engine.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 16-04-2025

#include <iostream>
#include "engine.h"

playerstate::playerstate()
	: 	health(0),
		comboCount(0),
		position{0, 0},
		directionalForce{0, 0},
		walk(0),
		gravityScaling(1),
		damageScaling(0),
		action(actionList[0]),
		frame(0),
		inactionable(false),
		hasHit(false),
		hasBlocked(false),
		mirror(false)
{
	//no further initialization needed
}//playerstate

playerstate::playerstate(int player)
	:	health(400),
		comboCount(0),
		directionalForce{0, 0},
		walk(0),
		gravityScaling(1),
		damageScaling(1),
		action(actionList[0]),
		frame(0),
		inactionable(false),
		hasHit(false),
		hasBlocked(false),
		mirror(!player)
{
	position[1] = 0;
	if(player == 1)
	{
		position[0] = 0;
	} else {
		position[0] = 300;
	}//else
}//playerstate

gamestate::gamestate()
	:	player{playerstate(1), playerstate(2)},
		frame(0),
		finished(false)
{
	//no further initialization needed
}//gamestate

gamestate engine::getGamestate(int requestedState)
{
	return statecache[requestedState];
}//getGamestate

void engine::setGamestate(gamestate state)
{
	statecache[0] = state;
}//setGamestate

bool engine::getFinished()
{
	return statecache.front().finished;
}//getFinished

void engine::printGamestate(gamestate state)
{
	std::cout << "Frame: " << state.frame << " ";
	std::cout << "Finished: " << state.finished << " ";
	for(int i = 1; i <= 2; i++)
	{
		playerstate currentPlayer = state.player[i-1];
		std::cout << "Player: " << i << " ";
		std::cout << "Health: " << currentPlayer.health << " ";
		std::cout << "Combo count: " << currentPlayer.comboCount << " ";
		std::cout << "Position: " << currentPlayer.position[0] << ", " 
				  << currentPlayer.position[1] << " ";
		std::cout << "Directional force: " << currentPlayer.directionalForce[0] << ", " 
				  << currentPlayer.directionalForce[1] << " ";
		std::cout << "Walk: " << currentPlayer.walk << " ";
		std::cout << "Gravity scaling: " << currentPlayer.gravityScaling << " ";
		std::cout << "Damage scaling: " << currentPlayer.gravityScaling << " ";
		std::cout << "Action damage: " << currentPlayer.action.damage << " ";
		std::cout << "Frame: " << currentPlayer.frame << " ";
		std::cout << "Inactionable: " << currentPlayer.inactionable << " ";
		std::cout << "Has hit: " << currentPlayer.hasHit << " ";
		std::cout << "Has blocked: " << currentPlayer.hasBlocked << " ";
		std::cout << "Mirror: " << currentPlayer.mirror << " ";
	}//for
	std::cout << std::endl;
}//printGamestate

int engine::addInput(int player, json data)
{
	for(long unsigned int i = 0; i < data.size(); i++)
	{
		if(data[i]["Pressed"] != nullptr)
		{
			std::string buttonString = data[i]["Pressed"];
			char button = buttonString[1];
			std::string frameString = data[i]["Frame"];
			int inputFrame = std::stoi(frameString);
			inputList[player-1].push_back(std::make_pair(button, inputFrame));
		}//if
	}//for
	return 0;
}//initInput

void engine::setCurrentPlayer(int player)
{
	currentPlayer = player;
}//setCurrentPlayer

int engine::getCurrentPlayer()
{
	return currentPlayer;
}//getCurrentPlayer

int engine::manageInputs(int player)
{
	//add all the inputs from the input queue that have entered
	//the buffers frame/time limit if there are inputs in the input queue
	while(!inputList[player-1].empty() && 
		  inputList[player-1].front().second <= statecache.front().frame + 1)
	{
		statecache.front().processingInput[player-1].push_back(inputList[player-1].front());
		inputList[player-1].pop_front();
	}//while

	//remove all the inputs from the vector which have passed
	//the buffers frame/time limit if there are inputs in the buffer
	while(!statecache.front().processingInput[player-1].empty() && 
		  (statecache.front().processingInput[player-1].front().second) < 
		  (statecache.front().frame + 1 - framesInBuffer))
	{
		statecache.front().processingInput[player-1].pop_front();
	}//while

	return 0;
}//manageInputs

void engine::setFirstFrame()
{
	statecache.reserve(9);
	gamestate firstState;
	statecache.insert(statecache.begin(), firstState);
}//setFirstFrame

void engine::prepStatecache()
{
	//copy the current frame and add it at the begining, pushing all older frames back
	statecache.insert(statecache.begin(), statecache[0]);

	//if we are storing more then 8 frames, remove the oldest one
	if(statecache.size() > 8)
	{
		statecache.pop_back();
	}//if
}//prepStatecache

gamestate engine::framegen()
{
	tickUpFrame();
	tickMovement(); 
	checkActiveHitbox();
	applyHitEffects();
	setNextActions();
	statecache.front().finished = gameOver();
	return statecache.front();
}//framegen

//set the current players input back in the input list before rolling
void engine::pushInputBack()
{
	while(!statecache.front().processingInput[currentPlayer-1].empty() &&
		  statecache.front().processingInput[currentPlayer-1].back().second == statecache.front().frame)
	{
		inputList[currentPlayer-1].push_front(statecache.front().processingInput[currentPlayer-1].back());
		statecache.front().processingInput[currentPlayer-1].pop_back();
	}//while
}//pushInputBack

//roll the gamestate back by the amount of frames given
void engine::rollback(int rollbackFrames)
{
	for(int i = 0; i < rollbackFrames; i++)
	{
		pushInputBack();
		statecache.erase(statecache.begin());
	}//for
}//rollback

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
	int otherPlayer = player % 2;
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
	//both players lose their x_force by colliding
	statecache.front().player[targetPlayer].directionalForce[0] = 0;
	statecache.front().player[otherPlayer].directionalForce[0] = 0;
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
	//check which player is standing to the right and should be mirrored
	if(statecache.front().player[0].position[0] > statecache.front().player[1].position[0])
	{
		statecache.front().player[0].mirror = true;
		statecache.front().player[1].mirror = false;
	} else {
		statecache.front().player[0].mirror = false;
		statecache.front().player[1].mirror = true;
	}//else

	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//remember if they were in the air before this tick
		bool playerInAir =  statecache.front().player[i-1].position[1] > 0;
		//if they are, tick gravity on them
		if(playerInAir)
			applyGravity(i);
		tickForceOnPlayer(i);
		//if the player ended up under the ground
		if(statecache.front().player[i-1].position[1] < 0)
			statecache.front().player[i-1].position[1] = 0;
		//if the player went from air to ground this tick
		if(playerInAir && statecache.front().player[i-1].position[1] == 0)
		{
			setPlayerInactionable(i, false);
			statecache.front().player[i-1].action = actionList[0];
			statecache.front().player[i-1].frame = 0;
			statecache.front().player[i-1].comboCount = 0;
			statecache.front().player[i-1].damageScaling = 1;
			statecache.front().player[i-1].gravityScaling = 1;
			statecache.front().player[i-1].directionalForce[0] = 0;
			statecache.front().player[i-1].directionalForce[1] = 0;
		//if the player isn't in the air, they could walk
		}else if(!playerInAir)
		{
			statecache.front().player[i-1].position[0] += statecache.front().player[i-1].walk;
		}//if

		statecache.front().player[i-1].walk = 0;

	}//for

	//if the movement caused both players to collide
	if (detectCollision())
	{
   		// resolve based on relative position
    	if (statecache.front().player[0].position[0] < statecache.front().player[1].position[0])
		{
			resolveCollision(1);
		}else{
			resolveCollision(2);
		}//else
	}//if

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
	//store for each player if they have been hit this frame
	bool wasHit[2] = {false, false};
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//calculate values to index arrays
		//for player 1: targetPlayer = 0, otherPlayer = 1
		//for player 2: targetPlayer = 1, otherPlayer = 0
		int targetPlayer = i - 1;
		int otherPlayer = i % 2;
		//if the player has hit their move
		if(statecache.front().player[targetPlayer].hasHit == true)
		{
			//if the other player was blocking/holding back, didn't press an action button
			//and isn't in another move. Function calls adjust player values back to 1 and 2
			if(((getMovementButton(otherPlayer+1) == 4 && !statecache.front().player[otherPlayer].mirror) || 
			   (getMovementButton(otherPlayer+1) == 6 && statecache.front().player[otherPlayer].mirror)) &&
			   getActionButton(otherPlayer+1) == -1 && statecache.front().player[otherPlayer].action.damage == -1)
			{
				//On block, a player is only pushed back and not thrown up
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
				addForceToPlayer(otherPlayer+1, x_force, y_force);
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
	//check each player again, if they have hit and weren't hit set them actionable again and
	//set their action to idle. if they were hit, set their action to idle too
	for(int i = 1; i <= 2; i++)
	{
		if(statecache.front().player[i-1].hasHit && !wasHit[i-1])
		{
			statecache.front().player[i-1].inactionable = false;
			statecache.front().player[i-1].action = actionList[0];
			statecache.front().player[i-1].frame = 0;
		} else if(wasHit[i-1])
		{
			statecache.front().player[i-1].action = actionList[0];
			statecache.front().player[i-1].frame = 0;
		}//else
		statecache.front().player[i-1].hasHit = false;
	}//for
}//applyHitEffect

void engine::setNextActions()
{
	//for each player
	for(int i = 1; i <= 2; i++)
	{
		//if the player isn't inactionable, process their input
		if(statecache.front().player[i-1].inactionable == false)
		{
			int actionButton = getActionButton(i);
			int movementButton = getMovementButton(i);
			//if no action was pressed
			if(actionButton == -1)
			{
				//if the player isn't in the air
				if(statecache.front().player[i-1].position[1] == 0)
				{
					//if the movement is a jump
					if(movementButton == 7 || movementButton == 8 || movementButton == 9) 
					{	
						statecache.front().player[i-1].directionalForce[1] += jumpForce;
						//if the player wants to jump to the left
						if(movementButton == 7)
						{
							statecache.front().player[i-1].directionalForce[0] -= jumpSideForce;
						//if the player wants to jump to the right
						}else if(movementButton == 9)
						{
							statecache.front().player[i-1].directionalForce[0] += jumpSideForce;
						}//if
					
					//if the player wants to move to the left
					}else if(movementButton == 4)
					{
						statecache.front().player[i-1].walk = -movementAmount;

					//if the player wants to move to the right
					} else if(movementButton == 6){
						statecache.front().player[i-1].walk = movementAmount;
					}//else
				}//if	
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

bool engine::gameOver()
{
	if(statecache.front().player[0].health <= 0 || statecache.front().player[1].health <= 0 ||
	   statecache.front().frame >= maxFrames)
		return true;
	return false;
}//gameOver

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
	for(int i = (int)statecache.front().processingInput[player-1].size() - 1; i >= 0; --i)
    {
        char input = statecache.front().processingInput[player-1][i].first;

        if(input == 'u' || input == 'i' ||
           input == 'j' || input == 'k')
        {
            return keymapping.at(input);
        }//if
    }//for

    return -1;
}//getAction

int engine::getMovementButton(int player)
{
	for(int i = (int)statecache.front().processingInput[player-1].size() - 1; i >= 0; --i)
    {
        char input = statecache.front().processingInput[player-1][i].first;

        if(input == 'w' || input == 'a' ||
           input == 's' || input == 'd')
        {
            return keymapping.at(input);
        }//if
    }//for

    return -1;
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
	int otherPlayer = player % 2;

	attacker = statecache.front().player[targetPlayer].action;
	attackerLocation[0] = statecache.front().player[targetPlayer].position[0];
	attackerLocation[1] = statecache.front().player[targetPlayer].position[1];
	mirrorAttacker = statecache.front().player[targetPlayer].mirror;
	defender = statecache.front().player[otherPlayer].action;
	defenderLocation[0] = statecache.front().player[otherPlayer].position[0];
	defenderLocation[1] = statecache.front().player[otherPlayer].position[1];
	mirrorDefender = statecache.front().player[otherPlayer].mirror;

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

void engine::printInputBuffer(int player)
{
	std::cout << "Player " << player << " processing input list ";
	for(long unsigned int i = 0; i < statecache.front().processingInput[player-1].size(); i++)
	{
		std::cout << " Input: " << statecache.front().processingInput[player-1][i].first
				  << " Frame: " << statecache.front().processingInput[player-1][i].second;
	}//for
}