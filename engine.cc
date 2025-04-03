//engine.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 03-04-2025

#include "engine.h"

gamestate engine::getGamestate(int requestedState)
{
	return statecache[requestedState];
}//getGamestate