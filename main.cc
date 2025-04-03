//main.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 03-04-2025

#include <iostream>

int main(int argc, char * argv[])
{
	std::string exec_mode = argv[1];
	if(!((argc == 3 && exec_mode == "SIMULATE") || (argc == 4 && exec_mode == "EMULATE")))
	{
		std::cerr << "Incorrect program call" << std::endl;
		return -1;
	}
	return 0;
}//main