//action_list.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include "action_list.h" 

const actions actionList[] =
{
	{//Idle (0)
		/*damage*/ -1, /*damage scaling*/ -1, 
		/*uptime*/ -1, /*hitbox*/ {-1, -1}, /*hurtbox*/ {-1, -1},
		/*hitbox origin*/ 		{{-1, -1}, {-1, -1}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{-1, -1}, {-1, -1}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{0, 0}, {-1, -1}, {-1, -1}}, 
		/*hurtbox dimensions*/ 	{{53, 141}, {-1, -1}, {-1, -1}},
		/*launch angle*/ {-1, -1}, /*launch force*/ -1, /*block type*/ -1
	},	
	{//5P (1)
		/*damage*/ 26, /*damage scaling*/ 0.8, 
		/*uptime*/ 15, /*hitbox*/ {5, 8}, /*hurtbox*/ {0, 15},
		/*hitbox origin*/ 		{{85, 18}, {-1, -1}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{101, 30}, {-1, -1}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-14, 35}, {13, 64}, {58, 43}}, 
		/*hurtbox dimensions*/ 	{{99, 223}, {46, 30}, {139, 64}},
		/*launch angle*/ {1, 0.1}, /*launch force*/ 10, /*block type*/ 0
	},	
	{//5K (2)
		/*damage*/ 30, /*damage scaling*/ 0.7, 
		/*uptime*/ 20, /*hitbox*/ {7, 14}, /*hurtbox*/ {0, 20},
		/*hitbox origin*/ 		{{68, -47}, {68, -88}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{61, 41}, {114, 95}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-14, 50}, {86, -40}, {135, -80}}, 
		/*hurtbox dimensions*/ 	{{100, 242}, {50, 151}, {53, 111}},
		/*launch angle*/ {1, 0.1}, /*launch force*/ 20, /*block type*/ 2
	},	
	{//5S (3)
		/*damage*/ 42, /*damage scaling*/ 1, 
		/*uptime*/ 22, /*hitbox*/ {7, 12}, /*hurtbox*/ {0, 22},
		/*hitbox origin*/ 		{{118, 13}, {69, -15}, {44, -62}}, 
		/*hitbox dimensions*/ 	{{83, 26}, {133, 47}, {171, 132}}, 
		/*hurtbox origin*/ 		{{61, 20}, {32, -14}, {-45, -63}}, 
		/*hurtbox dimensions*/ 	{{151, 10}, {243, 75}, {264, 136}},
		/*launch angle*/ {1, 0.5}, /*launch force*/ 20, /*block type*/ 0
	},	
	{//5H (4)
		/*damage*/ 48, /*damage scaling*/ 0.9, 
		/*uptime*/ 38, /*hitbox*/ {12, 17}, /*hurtbox*/ {0, 38},
		/*hitbox origin*/ 		{{75, -25}, {75, -41}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{86, 16}, {126, 30}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-10, 15}, {85, 4}, {85, -54}}, 
		/*hurtbox dimensions*/ 	{{94, 138}, {26, 46}, {11, 72}},
		/*launch angle*/ {1, 0.1}, /*launch force*/ 30, /*block type*/ 0
	},	
	{//2P (5)
		/*damage*/ 22, /*damage scaling*/ 0.8, 
		/*uptime*/ 16, /*hitbox*/ {5, 8}, /*hurtbox*/ {0, 16},
		/*hitbox origin*/ 		{{87, -23}, {-1, -1}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{87, 30}, {-1, -1}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-15, 10}, {87, 0}, {-1, -1}}, 
		/*hurtbox dimensions*/ 	{{100, 150}, {96, 61}, {-1, -1}},
		/*launch angle*/ {0.25, 0.1}, /*launch force*/ 20, /*block type*/ 0
	},	
	{//2K (6)
		/*damage*/ 26, /*damage scaling*/ 0.7, 
		/*uptime*/ 19, /*hitbox*/ {6, 9}, /*hurtbox*/ {0, 19},
		/*hitbox origin*/ 		{{54, -56}, {54, -89}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{62, 33}, {126, 48}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-23, 10}, {84, -49}, {121, -84}}, 
		/*hurtbox dimensions*/ 	{{106, 145}, {38, 87}, {64, 54}},
		/*launch angle*/ {0.5, 0.25}, /*launch force*/ 20, /*block type*/ 2
	},
	{//2S (7)
		/*damage*/ 32, /*damage scaling*/ 0.9, 
		/*uptime*/ 32, /*hitbox*/ {11, 12}, /*hurtbox*/ {0, 32},
		/*hitbox origin*/ 		{{54, -43}, {54, -53}, {54, -73}}, 
		/*hitbox dimensions*/ 	{{43, 10}, {107, 19}, {147, 26}}, 
		/*hurtbox origin*/ 		{{-11, 8}, {83, -38}, {101, -48}}, 
		/*hurtbox dimensions*/ 	{{93, 111}, {18, 66}, {64, 55}},
		/*launch angle*/ {0.75, 0.25}, /*launch force*/ 30, /*block type*/ 2
	},
	{//2H (8)
		/*damage*/ 40, /*damage scaling*/ 0.9, 
		/*uptime*/ 43, /*hitbox*/ {11, 14}, /*hurtbox*/ {0, 43},
		/*hitbox origin*/ 		{{79, 144}, {120, 165}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{63, 157}, {62, 157}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-14, 109}, {69, 113}, {-1, -1}}, 
		/*hurtbox dimensions*/ 	{{118, 242}, {125, 178}, {-1, -1}},
		/*launch angle*/ {0.5, 0.5}, /*launch force*/ 30, /*block type*/ 0
	},
	{//6P (9)
		/*damage*/ 34, /*damage scaling*/ 0.9, 
		/*uptime*/ 35, /*hitbox*/ {9, 14}, /*hurtbox*/ {0, 35},
		/*hitbox origin*/ 		{{88, 29}, {125, 17}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{37, 12}, {119, 72}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-26, -108}, {-26, -126}, {-1, -1}}, 
		/*hurtbox dimensions*/ 	{{171, 18}, {191, 64}, {-1, -1}},
		/*launch angle*/ {1, 0.5}, /*launch force*/ 20, /*block type*/ 0
	},
	{//6K (10)
		/*damage*/ 40, /*damage scaling*/ 0.9, 
		/*uptime*/ 37, /*hitbox*/ {25, 26}, /*hurtbox*/ {0, 37},
		/*hitbox origin*/ 		{{44, 0}, {-1, -1}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{148, 75}, {-1, -1}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-19, 35}, {102, 13}, {-34, -47}},
		/*hurtbox dimensions*/ 	{{149, 81}, {99, 98}, {135, 139}},
		/*launch angle*/ {1, 0.25}, /*launch force*/ 20, /*block type*/ 0
	},
	{//6H (11)
		/*damage*/ 52, /*damage scaling*/ 0.9, 
		/*uptime*/ 44, /*hitbox*/ {15, 18}, /*hurtbox*/ {0, 44},
		/*hitbox origin*/ 		{{99, -41}, {-1, -1}, {-1, -1}}, 
		/*hitbox dimensions*/ 	{{144, 74}, {-1, -1}, {-1, -1}}, 
		/*hurtbox origin*/ 		{{-11, 26}, {110, -24}, {-11, -60}},
		/*hurtbox dimensions*/ 	{{120, 85}, {58, 37}, {110, 85}},
		/*launch angle*/ {1, 0.5}, /*launch force*/ 20, /*block type*/ 0
	}
};//actionList

const std::map<std::array<short, 2>, actions> buttonMapping =
{
	//idle
	{{-1, -1}, actionList[0]},
	//5P
	{{-1, 0}, actionList[1]},
	//5K
	{{-1, 1}, actionList[2]},
	//5S
	{{-1, 2}, actionList[3]},
	//5H
	{{-1, 3}, actionList[4]},
	//2P
	{{2, 0}, actionList[5]},
	//2K
	{{2, 1}, actionList[6]},
	//2S
	{{2, 2}, actionList[7]},
	//2H
	{{2, 3}, actionList[8]},
	//6P
	{{6, 0}, actionList[9]},
	//6K
	{{6, 1}, actionList[10]},
	//6H
	{{6, 3}, actionList[11]}
};//buttonMapping