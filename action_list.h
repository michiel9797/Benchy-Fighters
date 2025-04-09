//action_list.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 09-04-2025

#ifndef ActionListH
#define ActionListH

#include <cstddef>
#include <tuple>

//information needed to define a move
struct actions
{
	//for any value, if they are unused they will be set to -1

	//the full damage a move deals
	int damage;
	//the percentage by which the damage of future moves in the combo
	//is decreased. This stacks multiplicatively with a minimum of 1 damage
	float damage_scaling;
	//the inputs required to execute the move. It works as follows:
	//the first 3 numbers are reserved for the required input directions
	//given the following numpad layout:
	//7 8 9
	//4 5 6
	//1 2 3
	//the number corresponding to the direction the player needs to have pressed
	//is encoded, in order of what order the directions need to have been pressed in
	//the last number stores what action button needs to have been pressed
	//0: (P)unch, 1: (K)ick, 2: (S)lash, 3: (H)eavy (S)lash
	int inputs[2];
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
extern const int actionCount;

#endif