#ifndef STATE_HPP_
#define STATE_HPP_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <stdint.h>
#include <limits>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <array>

using namespace std;

#include "Timer.hpp"
#include "Bug.hpp"
#include "Location.hpp"
#include "Ant.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "Definitions.hpp"
#include "GameData.hpp"

/*
 struct to store current state information
 */
struct State
{
    /*
     Variables
     */

    bool gameover;
	bool first_enemy_spotted;
	//vector<int> times_not_seen;
	vector<int> times_seen;
	vector<int> times_not_seen;
	vector<int> times_visited;
	
	vector< array<int, NO_OF_DIRS_> > neighbor_index;
	
	vector< int > combat_total_influence;
	vector< vector<int> > combat_influence;
	vector< vector<int> > combat_fighting;
	vector< vector<int> > combat_status;
	
	vector<int> water;	
	vector<int> explored;
	vector<int> visible;
	vector<int> invisible;
	vector<int> inexplored;
	vector<int> food;
	vector<int> my_hills_vector;
	vector<int> enemy_hills_vector;

	vector<Ant*> ant_map;
	vector<Ant*> all_ants_list;
	vector<Ant> my_ants;
	vector<Ant> enemy_ants;
	//unordered_map<int, Ant> incoming_ants;

	unordered_map<int,int> enemy_hills;
	unordered_set<int> my_hills;

    vector< pair<int, int> > spawn_mask;
    vector< pair<int, int> > view_mask;
	vector< pair<int, int> > combat_1_mask;
	vector< pair<int, int> > combat_2_mask;
	vector< pair<int, int> > combat_mask;
    /*
     Functions
     */
    State();
    ~State();

    void setup();
    void afterTurn();

    void sendMove(Ant &ant, const int &direction);
    bool isWaterAt(const int &hash) const;
	bool isFoodAt(const int &hash) const;
	bool isAntAtDest(const int &hash) const;
	bool isVisible(const int &hash) const;
	bool isExplored(const int &hash) const;
	bool isExpired(const int &thash) const;

    bool isPassable(const int &hash) const;

	void resetAntMap();
	void resetCombat();
	
	void calculateCombatMaps();

	void updatePlayers();
    void updateVision();
	void updateAnts();
    void cleanFood();
    void updateHills();

    void update();
};

//std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

//constructor
inline State::State()
{
    gameover = 0;
	first_enemy_spotted = false;
    GameData::turn = 0;
    bug_log.open("./debug.txt");
	
	enemy_hills = unordered_map<int, int>();
	my_hills = unordered_set<int>();				
	
	spawn_mask = vector< pair<int,int> >();
    view_mask = vector< pair<int,int> >();
	combat_1_mask = vector< pair<int,int> >();
	combat_2_mask = vector< pair<int,int> >();
}

//deconstructor
inline State::~State()
{
    bug_log.close();
}

//sets the state up
inline void State::setup()
{
	bug_log << "State::setup()" << endl;
    srand(GameData::seed); 
	EXPIRATION_PERIOD = 10;// (GameData::rows + GameData::cols) / 10;
	
	resetAntMap();
	
	neighbor_index = vector< array<int, NO_OF_DIRS_> >(GameData::rows*GameData::cols, array<int,NO_OF_DIRS_>());		
	
	my_hills_vector = vector<int>(GameData::rows * GameData::cols, 0);
	enemy_hills_vector = vector<int>(GameData::rows * GameData::cols, 0);
	
	all_ants_list = vector<Ant*>();
	my_ants = vector<Ant>();
	enemy_ants = vector<Ant>();
	
	water = vector<int>(GameData::rows * GameData::cols, 0);
	explored = vector<int>(GameData::rows * GameData::cols, 0);
	visible = vector<int>(GameData::rows * GameData::cols, 0);
	inexplored = vector<int>(GameData::rows * GameData::cols, 1);
	invisible = vector<int>(GameData::rows * GameData::cols, 1);
	food = vector<int>(GameData::rows * GameData::cols, 0);	
	
	times_seen = vector<int>(GameData::rows * GameData::cols, 0);
	times_not_seen = vector<int>(GameData::rows * GameData::cols, EXPIRATION_PERIOD);
	times_visited = vector<int>(GameData::rows * GameData::cols, 0);
	
	combat_total_influence = vector<int>(GameData::rows * GameData::cols, 0);
	combat_influence = vector< vector<int> >(CURRENT_NO_OF_PLAYERS, vector<int>(GameData::rows * GameData::cols, 0));
	combat_fighting = vector< vector<int> >(CURRENT_NO_OF_PLAYERS, vector<int>(GameData::rows * GameData::cols, INT_MAX));;
	combat_status = vector< vector<int> >(CURRENT_NO_OF_PLAYERS, vector<int>(GameData::rows * GameData::cols, COMBAT_SAFE));;
	
    genRangeMask(GameData::spawnradius2, spawn_mask);
    genRangeMask(GameData::viewradius2, view_mask);
	genCombatMask(GameData::attackradius2, 1, combat_1_mask);
	genCombatMask(GameData::attackradius2, 2, combat_2_mask);
	genRangeMask(GameData::attackradius2, combat_mask);
		
	for(int h = 0; h < GameData::rows * GameData::cols; h++) {
		Location loc = hashToLocation(h);
		
		for(int d = 0; d < NO_OF_DIRS_; d++) {
			//TODO doesnt work
			Location tloc = getModuloLocation(loc.row + AIM_WITH_NONE[d][0], loc.col + AIM_WITH_NONE[d][1]); //getLocation(loc, d);
			int thash = locationToHash(tloc);
			
			this->neighbor_index[h][d] = thash;
		}
	}
}

//after turn cleanup TODO
inline void State::afterTurn()
{
	cleanFood();
	this->my_ants.clear();
	this->enemy_ants.clear();
	this->all_ants_list.clear();
	resetAntMap();
	resetCombat();
}

inline void State::resetCombat() {
	fill(combat_total_influence.begin(), combat_total_influence.end(), 0);
	for(int p = 0; p < CURRENT_NO_OF_PLAYERS; p++) {
		fill(combat_fighting[p].begin(), combat_fighting[p].end(), INT_MAX);
		fill(combat_influence[p].begin(), combat_influence[p].end(), 0);
		fill(combat_status[p].begin(), combat_status[p].end(), COMBAT_SAFE);
	}
}

inline void State::calculateCombatMaps() {
	bug_log << "State::calculateCombatMaps()" << endl;
	Location aloc,tloc;
	int nrow, ncol, ahash, thash, enemies;	
	
	unordered_map<int,bool> visited = unordered_map<int,bool>();
	
	//calc attack values
	for(auto aiter = all_ants_list.begin(); aiter != all_ants_list.end(); ++aiter) {
		Ant *a = *aiter;
		//aloc = hashToLocation(a->pos_hash);
		//for every tile in combat range +1
		for(int d = 0; d < NO_OF_DIRS_; d++) {
			ahash = neighbor_index[a->pos_hash][d];			
			if(isPassable(ahash) || ahash == a->pos_hash) {
				aloc = hashToLocation(ahash);
				for(auto piter = combat_mask.begin(); piter != combat_mask.end(); ++piter) {
					pair<int,int> &p = *piter;
					nrow = aloc.row + p.first;
					ncol = aloc.col + p.second;
					tloc = getModuloLocation(nrow, ncol);
					thash = locationToHash(tloc);
					if(!visited[thash]) {						
						combat_influence[a->owner][thash]++;
						combat_total_influence[thash]++;
					}
					visited[thash] = true;
				}
			}
		}
		visited.clear();
	}

	//calculate how many enemies the best ant in combat zone is fighting
	for(auto aiter = all_ants_list.begin(); aiter != all_ants_list.end(); ++aiter) {
		Ant *a = *aiter;
		aloc = hashToLocation(a->pos_hash);
		//aloc = hashToLocation(a->pos_hash);
		//for every tile in combat range +1
		for(int d = 0; d < NO_OF_DIRS_; d++) {
			ahash = neighbor_index[a->pos_hash][d];
			enemies = combat_total_influence[ahash] - combat_influence[a->owner][ahash];
			
			if(isPassable(ahash) || ahash == a->pos_hash) {
				aloc = hashToLocation(ahash);
				
				for(auto piter = combat_mask.begin(); piter != combat_mask.end(); ++piter) {
					pair<int,int> &p = *piter;
					nrow = aloc.row + p.first;
					ncol = aloc.col + p.second;
					tloc = getModuloLocation(nrow, ncol);
					thash = locationToHash(tloc);
					
					
					if(enemies < combat_fighting[a->owner][thash]) {
						combat_fighting[a->owner][thash] = enemies;
					}
				}
			}
		}
	}

	int best_enemy;
	
	//calculate combat statii for own ants
	for(auto aiter = all_ants_list.begin(); aiter != all_ants_list.end(); ++aiter) {
		Ant &a = **aiter;
		for(int d = 0; d < NO_OF_DIRS_; d++) {
			ahash = neighbor_index[a.pos_hash][d];
			if(isPassable(ahash) || ahash == a.pos_hash) {
				//find best enemy
				enemies = combat_total_influence[ahash] - combat_influence[a.owner][ahash];			
				best_enemy = INT_MAX;
				//for every enemy
				for(int player = 0; player < CURRENT_NO_OF_PLAYERS; player++) {
					if(player != a.owner) {
						if(combat_fighting[player][ahash] < best_enemy) {
							//store it
							best_enemy = combat_fighting[player][ahash];
						}					
					}
				}				
				//if ant is better then enemy ants
				if(enemies < best_enemy) {
					//mark place as safe
					combat_status[a.owner][ahash] = COMBAT_SAFE;				
				} else if(enemies == best_enemy) {
					//if they are equal mark place as draw
					combat_status[a.owner][ahash] = COMBAT_DRAW;
				} else {
					//else ant is weaker than enemies .. mark as lost
					combat_status[a.owner][ahash] = COMBAT_LOST;
				}
			}
		}
	}
	
/*	
	int status;
	//print to visualizer
	for(Ant *a : all_ants_list) {
		ahash = a->pos_hash;

		for(int d = 0; d < NO_OF_DIRS_; d++) {
			thash = neighbor_index[ahash][d];
			tloc = hashToLocation(thash);
			
			status = combat_status[a->owner][thash];
			if(status == COMBAT_SAFE) {
				cout << "v setFillColor 0 255 0 0.5" << endl;
				cout << "v tile " << tloc.row << " " << tloc.col << endl;
			} else if(status == COMBAT_DRAW) {
				cout << "v setFillColor 255 255 0 0.5" << endl;
				cout << "v tile " << tloc.row << " " << tloc.col << endl;
			} else if(status == COMBAT_LOST) {
				cout << "v setFillColor 255 0 0 0.5" << endl;
				cout << "v tile " << tloc.row << " " << tloc.col << endl;				
			}
		}
	}
*/	
}


inline void State::resetAntMap() {
	this->ant_map = vector<Ant*>(GameData::rows*GameData::cols, NULL);
}

//outputs move information to the engine
inline void State::sendMove(Ant &ant, const int &direction)
{	
    Location loc = hashToLocation(ant.pos_hash);
	cout << "o " << loc.row << " " << loc.col << " " << DIRECTIONS[direction] << endl;
    //bug_log << "o " << ant.loc.row << " " << ant.loc.col << " " << DIRECTIONS[direction] << endl;
}

inline bool State::isPassable(const int &hash)  const {		
	return (!isWaterAt(hash) && !isFoodAt(hash) && !isAntAtDest(hash));
}

inline bool State::isAntAtDest(const int &hash) const {
	return (this->ant_map[hash] != NULL);
}

inline bool State::isWaterAt(const int &hash) const {
    return this->water[hash];
}

inline bool State::isFoodAt(const int &hash) const {
    return this->food[hash];
}

inline bool State::isVisible(const int &hash) const {
	return this->visible[hash];
}

inline bool State::isExplored(const int &hash) const {
	return this->explored[hash];	
}

inline bool State::isExpired(const int &hash) const {
	return (this->times_not_seen[hash] >= EXPIRATION_PERIOD);
}


inline void State::update()
{
	bug_log << "State::update()" << endl;
	updatePlayers();
	updateAnts();
    updateVision();
    //updateFood();
    updateHills();
	
	calculateCombatMaps();
}

inline void State::updatePlayers() {
	int old_size = combat_status.size();
	
	if(CURRENT_NO_OF_PLAYERS > old_size) {
		bug_log << "Adjusting players from: " << old_size << " to: " << CURRENT_NO_OF_PLAYERS << endl;
		for(int i = 0; i < CURRENT_NO_OF_PLAYERS-old_size; i++) {
			combat_status.push_back(vector<int>(GameData::rows * GameData::cols, COMBAT_SAFE));
			combat_fighting.push_back(vector<int>(GameData::rows * GameData::cols, INT_MAX));
			combat_influence.push_back(vector<int>(GameData::rows * GameData::cols, 0));
		}
	}
}


inline void State::updateAnts() {
	bug_log << "State::updateAnts()" << endl;
	//copy all ants to antmap
	for(auto aiter = my_ants.begin(); aiter != my_ants.end(); ++aiter) {
		Ant &ant = *aiter;
		this->times_visited[ant.pos_hash]++;
		this->ant_map[ant.pos_hash] = &ant;
		this->all_ants_list.push_back(&ant);
	}
	for(auto aiter = enemy_ants.begin(); aiter != enemy_ants.end(); ++aiter) {
		Ant &ant = *aiter;
		this->ant_map[ant.pos_hash] = &ant;
		this->all_ants_list.push_back(&ant);
	}
}

inline void State::updateHills()
{
	bug_log << "State::updateHills()" << endl;
	//remove lost hills
	auto ohiter = this->my_hills.begin();
	
	while(ohiter != this->my_hills.end()) {
		if(this->ant_map[*ohiter] != NULL && this->ant_map[*ohiter]->owner != 0) {
			this->my_hills_vector[*ohiter] = 0;
			this->my_hills.erase(ohiter++);
		} else {
			++ohiter;
		}
	}
	
	auto ehiter = this->enemy_hills.begin();
	//remove razed hills
	while(ehiter != this->enemy_hills.end()) {		
		if(this->ant_map[ehiter->first] != NULL && this->ant_map[ehiter->first]->owner != ehiter->second) {
			this->enemy_hills_vector[ehiter->first] = 0;
			this->enemy_hills.erase(ehiter++);
		} else {
			++ehiter;
		}
	}
}

inline void State::cleanFood()
{
	bug_log << "State::cleanFood()" << endl;
	Location loc, tloc;
	int thash;
	
	for(int h = 0; h < GameData::rows*GameData::cols; h++) {		
		loc = hashToLocation(h);
		if(this->food[h] == 1) {
			for(auto siter = this->spawn_mask.begin(); siter != this->spawn_mask.end(); ++siter) {
				tloc = getModuloLocation(loc.row + siter->first, loc.col + siter->second);
				thash = locationToHash(tloc);
				if(this->ant_map[thash] != NULL) {
					this->food[h] = 0;
					break;
				}
			}
		}
	}
}

inline void State::updateVision()
{
    bug_log << "State::updateVision()" << endl;
    int row, col, hash;
    Location aloc;
	
	//clear visible tiles
	fill(visible.begin(), visible.end(), 0);
	fill(invisible.begin(), invisible.end(), 1);

	for(auto aiter = my_ants.begin(); aiter != my_ants.end(); ++aiter) {
		Ant &ant = *aiter;
		for(auto piter = view_mask.begin(); piter != view_mask.end(); ++piter) {
			pair<int,int> &mask = *piter;

			aloc = hashToLocation(ant.pos_hash);
            row = aloc.row + mask.first;
            col = aloc.col + mask.second;
            aloc = getModuloLocation(row, col);
			hash = locationToHash(aloc);
				
			this->visible[hash] = 1;
			this->invisible[hash] = 0;
			this->inexplored[hash] = 0;
			this->explored[hash] = 1;
        }
    }

	
	for(int h = 0; h < GameData::rows * GameData::cols; h++) {
		if(!isVisible(h)) {
			this->times_not_seen[h]++;
		} else {
			this->times_seen[h]++;
			this->times_not_seen[h] = 0;
		}
	}
}

/*
 This is the output function for a state. It will add a char map
 representation of the state to the output stream passed to it.

 For example, you might call "cout << state << endl;"
 */

inline ostream& operator<<(ostream &os, const State &state)
{
	for(int r = 0; r < GameData::rows; r++) {
		for(int c = 0; c < GameData::cols; c++) {
			Location loc = Location(r,c);
			int hash = locationToHash(loc);
			if(state.ant_map[hash] != NULL) {
				os << state.ant_map[hash]->owner;
			} else if(state.water[hash] == 1) {
				os << "~";
			} else if(state.my_hills.find(hash) != state.my_hills.end()) {
				os << "H";
			} else if(state.enemy_hills.find(hash) != state.enemy_hills.end()) {
				os << "G";
			}  else if(state.food[hash] == 1) {
				os << "f";
			} else if(state.explored[hash] == 0) {
				os << "?";
			} else {
				os << ".";
			}
		}
		os << endl;
	}
	os << endl;
	return os;
}

//input function
inline istream& operator>>(istream &is, State &state)
{
    int row, col, player;
    string inputType, junk;
	
    //finds out which turn it is
	while(is >> inputType) {
        if (inputType == "end") {
            state.gameover = true;
            break;
        } else if (inputType == "turn") {
            is >> GameData::turn;
            break;
        } else { 
            //unknown line			
            getline(is, junk);			
		}		
    }

    if (GameData::turn == 0) {
        //reads game parameters
        while (is >> inputType) {
            if (inputType == "loadtime")
                is >> GameData::loadtime;
            else if (inputType == "turntime")
                is >> GameData::turntime;
            else if (inputType == "rows")
                is >> GameData::rows;
            else if (inputType == "cols")
                is >> GameData::cols;
            else if (inputType == "turns")
                is >> GameData::turns;
            else if (inputType == "player_seed")
                is >> GameData::seed;//state.seed;
            else if (inputType == "viewradius2") {
                is >> GameData::viewradius2;
                GameData::viewradius = sqrt(GameData::viewradius2);
            } else if (inputType == "attackradius2") {
                is >> GameData::attackradius2;
            } else if (inputType == "spawnradius2") {
                is >> GameData::spawnradius2;
            } else if (inputType == "ready") //end of parameter input
			{
                break;
            } else
                //unknown line
                getline(is, junk);
        }
    } else {
        //reads information about the current turn
        while (is >> inputType) {
            if (inputType == "w") { //water square
                is >> row >> col;
				int hash = locationToHash(Location(row,col));
				//state.water.insert(hash);
				state.water[hash] = 1;
				//state.explored.insert(hash);
            } else if (inputType == "f") { //food square
                is >> row >> col;
                int fhash = locationToHash(Location(row, col));
                //state.food.insert(fhash);
				state.food[fhash] = 1;
            } else if (inputType == "a") { //live ant square
                is >> row >> col >> player;
				if(player >= CURRENT_NO_OF_PLAYERS) {
					bug_log << "player >= CURRENT_NO_OF_PLAYERS (a)" << endl;
					CURRENT_NO_OF_PLAYERS = player+1;
				}
				Location aloc = Location(row, col);
				//int ahash = locationToHash(aloc);                
                if(player == 0) {
					state.my_ants.push_back(Ant(aloc, player));
                } else {
					/*
					if(!state.first_enemy_spotted) {
						CHASE_RANGE = max(min(CHASE_RANGE, GameData::turn/10), CHASE_MAX_RANGE)
						state.first_enemy_spotted = true;
					}
					*/
					state.enemy_ants.push_back(Ant(aloc, player));
                }
            } else if (inputType == "d") { //dead ant square
                is >> row >> col >> player;
                //IGNORE DEAD ANTS
            } else if (inputType == "h") {
                is >> row >> col >> player;
				if(player >= CURRENT_NO_OF_PLAYERS) {
					bug_log << "player >= CURRENT_NO_OF_PLAYERS (h)" << endl;
					CURRENT_NO_OF_PLAYERS = player+1;
				}
				int hhash = locationToHash(Location(row,col));
                if(player == 0) {
					state.my_hills.insert(hhash);
					state.my_hills_vector[hhash] = 1;
				} else {
					state.enemy_hills_vector[hhash] = 1;
					state.enemy_hills[hhash] = player;
				}
            }
			/* else if (inputType == "players") //player information
                is >> GameData::no_of_players;
            else if (inputType == "scores") { //score information
                state.scores = vector<double>(GameData::no_of_players, 0.0);
                for (int p = 0; p < GameData::no_of_players; p++)
                    is >> state.scores[p];
            }*/ 
			else if (inputType == "go") { //end of turn input
                if (state.gameover)
                    is.setstate(std::ios::failbit);
                break;
            } else
                //unknown line
                getline(is, junk);
        }
    }
	
	ttt.start();

    return is;
}

#endif //STATE_H_
