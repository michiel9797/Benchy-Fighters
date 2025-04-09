//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 09-04-2025

#include <iostream>
#include "engine.h"

int main(int argc, char * argv[])
{
	std::string exec_mode = argv[1];
	if(!((argc == 4 && exec_mode == "SIMULATE") || (argc == 5 && exec_mode == "EMULATE")))
	{
		std::cerr << "Incorrect program call" << std::endl;
		return -1;
	}
	engine gameEngine;

	/*
	if(gameEngine.checkHit(1))
		std::cout << "Nah" << std::endl;

	gameEngine.setAction(2, 3);

	if(gameEngine.checkHit(2))
		std::cout << "Yeh" << std::endl;
	*/

	//gameEngine.testGravity(1, 8);



	return 0;
}//main