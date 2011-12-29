/*
 * Statistics.hpp
 *
 *  Created on: Nov 2, 2011
 *      Author: dlb
 */

#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include "Timer.hpp"

struct GameData {
    static int rows;
    static int cols;
    static int turns;
    static int64_t seed;
    static int turn;
    static int no_of_players;
	static int attackradius2;
	static int spawnradius2;
	static float viewradius;
	static int viewradius2;
	static int loadtime;
	static int turntime; 
};

int GameData::rows = 0;
int GameData::cols = 0;
int GameData::turns = 1000;
int64_t GameData::seed = 0;
int GameData::turn = 0;
int GameData::no_of_players;
int GameData::attackradius2 = 5;
int GameData::spawnradius2 = 1;
int GameData::viewradius2 = 77;
float GameData::viewradius = 8.774964387f;
int GameData::loadtime = 3000; //ms
int GameData::turntime = 500; //ms
#endif /* GAMEDATA_HPP_ */
