//action_list.h
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

//Last edited: 05-04-2025

#ifndef ActionListH
#define ActionListH

#include <tuple>

struct projectiles
{
	//the full damage a projectile deals
	int damage;
	//how long a projectile lasts
	int lifetime;
	//the projectiles hitbox
	int hitbox;
	//the projectiles speed
	int speed;
	//the projectiles direction
	std::tuple <float, float> direction;
	//when the projectile was spawned
	float spanwTime;
};//projectiles

//information needed to define a move
struct actions
{
	//for any value, if they are unused they will be set to -1

	//the full damage a move deals
	int damage;
	//the inputs required to execute the move. It works as follows:
	//the first 3 numbers are reserved for the required input directions
	//given the following numpad layout:
	//7 8 9
	//4 5 6
	//1 2 3
	//the number corresponding to the direction the player needs to have held
	//is encoded, in order of what order the directions need to have been held in
	//the last number stores what action button needs to have been pressed
	//0: punch, 1: kick, 2: slash, 3: heavy slash
	int inputs[4];
	//the full amount of frames the move is active for, including its recovery
	int uptime;
	//the frames at which the first hitbox set appears and dissapears
	int first_hitbox[2];
	//the frame at which a possible second hitbox set appears and dissapears
	int second_hitbox[2];
	//the frame at which the first hurtbox set appears and dissapears
	int first_hurtbox[2];
	//the frame at which a possible second hurtbox set appears and dissapears
	int second_hurtbox[2];
	//the top left spot where each of the max 3 hitboxes appear
	//0-2 encode the first set of hitboxes, 3-5 encode the second set
	int hitbox_origin[6];
	//the lenght and width of the max 3 hitboxes
	//0-2 encode the first set of hitboxes, 3-5 encode the second set
	int hitbox_dimensions[6][2];
	//the top left spot where each of the max 3 hurtboxes appear
	//0-2 encode the first set of hitboxes, 3-5 encode the second set
	int hurtbox_origin[6];
	//the lenght and width of the max 3 hurtboxes
	//0-2 encode the first set of hitboxes, 3-5 encode the second set
	int hurtbox_dimensions[6][2];
	//the angle at which a move launches
	std::tuple <float, float> launch_angle;
	//the force with which a move launches
	float launch_force;
	//if the move can be used grounded or aerial
	bool is_aerial;
	//the projectile this move spawns, if any
	projectiles projectile;
};//actions

extern const actions actionList[];
extern const int actionCount;

#endif