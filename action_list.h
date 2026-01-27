//action_list.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#ifndef ActionListH
#define ActionListH

#include <cstddef>
#include <array>
#include <tuple>
#include <map>

//information needed to define a move
struct actions
{
	//for any value, if they are unused they will be set to -1

	//the full damage a move deals
	int damage;
	//the percentage by which the damage of future moves in the combo
	//is decreased. This stacks multiplicatively with a minimum of 1 damage
	float damage_scaling;
	//the full amount of frames the move is active for, including its recovery
	int uptime;
	//the frames at which the hitbox set appears and dissapears
	int hitbox[2];
	//the frames at which the hurtbox set appears and dissapears
	int hurtbox[2];
	//the top left spot relative to the player where each of the max 3 hitboxes appear
	int hitbox_origin[3][2];
	//the lenght and width of the max 3 hitboxes
	int hitbox_dimensions[3][2];
	//the top left spot relative to the player where each of the max 3 hurtboxes appear
	int hurtbox_origin[3][2];
	//the lenght and width of the max 3 hurtboxes
	int hurtbox_dimensions[3][2];
	//the angle at which a move launches
	float launch_angle[2];
	//the force with which a move launches
	float launch_force;
	//whether the move hits high, low or neutral
	//0: neutral, 1: high, 2: low
	int block_type;
};//actions

extern const actions actionList[];

//the inputs required to execute the move mapped to their action. It works as follows:
//the first number is reserved for the required input directions
//given the following numpad layout:
//7 8 9
//4 5 6
//1 2 3
//the number corresponding to the direction the player needs to have pressed is encoded
//-1 is encoded if a direction is not required for the move
//the last number stores what action button needs to have been pressed
//0: (P)unch, 1: (K)ick, 2: (S)lash, 3: (H)eavy Slash
extern const std::map<std::array<int, 2>, actions> buttonMapping;

#endif