/*
 * Definitions.hpp
 *
 *  Created on: Oct 28, 2011
 *      Author: dlb
 */

#ifndef DEFINITIONS_HPP_
#define DEFINITIONS_HPP_

#include <limits>
using namespace std;

/*
 constants
 */
const static int NO_OF_DIRS = 4;
const static int NO_OF_DIRS_ = 5;
const static char DIRECTIONS[NO_OF_DIRS_] = { 'N', 'E', 'S', 'W', '_' };
const static int AIM_WITH_NONE[NO_OF_DIRS_][2] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 }, {0, 0} }; //{N, E, S, W, NONE}
const static int AIM[NO_OF_DIRS][2] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } }; //{N, E, S, W}

const static int MAX_FOOD_DIST = 50;
const static int MAX_DEFENSE_DIST = 15;
const static int MAX_RAZE_DIST = 150;

const static double FOOD_COEFFICIENT = 100.0;
const static double INVISIBLE_COEFFICIENT = 50.0;

const static int PURPOSE_NONE = -1;
const static int PURPOSE_EXPLORE = 0;
const static int PURPOSE_COLLECT = 1;
const static int PURPOSE_RAZE = 2;
const static int PURPOSE_DEFENSE = 3;
const static int PURPOSE_COMBAT = 4;


const static double WEIGHT_FOOD = 1.0;
const static double WEIGHT_RAZE = 1.0e10;
const static double WEIGHT_EXPIRED = 1.0;
const static double WEIGHT_HUNT = 100000.0;

static int EXPIRATION_PERIOD = 50;

static int CURRENT_NO_OF_PLAYERS = 1;

const static int COMBAT_SAFE = 0;
const static int COMBAT_DRAW = 1;
const static int COMBAT_LOST = 2;

const static float RAZER_MAX = 0.5;
const static float FIGHTER_MAX = 0.7;

const static int CHASE_MIN_RANGE = 10;
const static int CHASE_MAX_RANGE = 20;
//static int CHASE_RANGE = 10;

static Timer ttt;

const static int INT_MAX = std::numeric_limits<int>::max();

#endif /* DEFINITIONS_HPP_ */
