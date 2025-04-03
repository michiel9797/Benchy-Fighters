//engine.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 03-04-2025

#ifndef EngineH
#define EngineH

#include <tuple>

struct playerstate
{
	int health;
	float position;
	std::tuple <float, float> momentum;
	float gravity;
	float damageScaling;
	int action;
	bool knockedDown;
};//playerstate

struct gamestate
{
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

};//engine

#endif