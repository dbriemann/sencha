#ifndef BOT_HPP_
#define BOT_HPP_

#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
using namespace std;

#include "Algo.hpp"
#include "State.hpp"
#include "Ant.hpp"
#include "Definitions.hpp"
#include "Option.hpp"
#include "WanderlustMap.hpp"

/*
 This struct represents your bot in the game of Ants
 */
struct Bot
{
    State state;
	vector<Option> options;
	vector<int> ignore_locs;
	unordered_map< int, vector<int> > food_distmaps;
	vector<int> raze_map;
	vector<int> defense_map;
	vector<int> explore_map;
	WanderlustMap wanderlust_map;

    Bot();
	
	void processMoves();
	
	void addFoodOptions();
	void addRazeOptions();
	void addWanderlustOptions();
	void addCombatOptions();
	void addExploOptions();
	void breakStalemates();
	
    void playGame(); //plays a single game of Ants

	void precalculate();
    void setup();
    void cleanUp();
    void endTurn(); //indicates to the engine that it has made its moves
};

//constructor
inline Bot::Bot()
{
	options = vector<Option>();
	wanderlust_map = WanderlustMap();
}

/*
 * sets up data which needs information of state first
 */
inline void Bot::setup()
{
	bug_log << "Bot::setup()" << endl;
	ignore_locs = vector<int>(GameData::rows*GameData::cols, 0);
	food_distmaps = unordered_map<int, vector<int> >();
	raze_map = vector<int>(GameData::rows*GameData::cols, -1);
	defense_map = vector<int>(GameData::rows*GameData::cols, -1);
	explore_map = vector<int>(GameData::rows*GameData::cols, -1);
}

inline void Bot::precalculate()
{
	//generate hill defense map
	genMultiDistMap(state.my_hills_vector, defense_map, 200 , state, 1);
	
	//generate hill raze map
	genMultiDistMap(state.enemy_hills_vector, raze_map, 200 , state, 10);
	
	//generate explore map
	genMultiDistMap(state.invisible, explore_map, 200, state, 3);
	
	//generate food maps
	for(int h = 0; h < GameData::rows*GameData::cols; h++) {
		if(state.isFoodAt(h)) {
			food_distmaps[h] = vector<int>(GameData::rows*GameData::cols, -1);
			genDistMap(h, food_distmaps[h], MAX_FOOD_DIST, state, 3);
		}
	}	
}

inline void Bot::breakStalemates() {
	//if mass is overwhelming
	int nrow, ncol, thash, ant_count, empty_count;
	Location aloc, tloc;
	
	for(auto a = state.my_ants.begin(); a != state.my_ants.end(); ++a) {
		ant_count = 0;
		empty_count = 0;
		aloc = hashToLocation(a->pos_hash);

		for(auto mask = state.combat_2_mask.begin(); mask != state.combat_2_mask.end(); ++mask) {
			pair<int,int> &m = *mask;
			nrow = aloc.row + m.first;
			ncol = aloc.col + m.second;
			tloc = getModuloLocation(nrow, ncol);
			thash = locationToHash(tloc);
			
			if(state.ant_map[thash] == NULL && state.water[thash] == 0) {
				empty_count++;
			} else if(state.ant_map[thash] != NULL && state.ant_map[thash]->owner == 0) {
				ant_count++;
			}
		}
		//TODO
	}
}

inline void Bot::addFoodOptions() {
	int score;
	int safety;
	int thash;
	
	for(auto a = state.my_ants.begin(); a != state.my_ants.end(); ++a) {
		if(!a->has_moved) {
			for(auto dmiter = food_distmaps.begin(); dmiter != food_distmaps.end(); ++dmiter) {
				for(int d = 0; d < NO_OF_DIRS_; d++) {
					thash = state.neighbor_index[a->pos_hash][d];
					score = dmiter->second[thash];
					safety = state.combat_status[0][thash];
					
					if(safety <= COMBAT_DRAW) {
						safety = COMBAT_SAFE; //allow ant exchanges when collecting food
					}
					if(score > 0 && safety < COMBAT_LOST) {
						this->options.push_back(Option(&(*a), d, dmiter->first, score, safety, PURPOSE_COLLECT));
					}
				}			
			}
		}
	}
}


inline void Bot::addCombatOptions() {
	int thash,safety;
	double score;//,prescore;
	//bool is_fighting;
	Location tloc,eloc,hloc;
	hloc = hashToLocation(*state.my_hills.begin());
	vector<int> enemy_map = vector<int>(GameData::rows*GameData::cols, -1);
	int maxrange = max(CHASE_MIN_RANGE, min(CHASE_MAX_RANGE, GameData::turn / 20));
	
	for(auto e = state.enemy_ants.begin(); e != state.enemy_ants.end(); ++e) {
		eloc = hashToLocation(e->pos_hash);
		genDistMap(e->pos_hash, enemy_map, maxrange, state, 20);
		
		//prescore = (1.0 / (state.combat_total_influence[e->pos_hash] - state.combat_influence[0][e->pos_hash])) / 100;
		
		for(auto a = state.my_ants.begin(); a != state.my_ants.end(); ++a) {
			if(!a->has_moved) {				
				for(int d = 0; d < NO_OF_DIRS_; d++) {
					thash = state.neighbor_index[a->pos_hash][d];
					if(enemy_map[thash] > 0) {
						tloc = hashToLocation(thash);
						//score = enemy_map[thash];

						score = defense_map[e->pos_hash] * defense_map[e->pos_hash] + enemy_map[thash];
						
						safety = state.combat_status[0][thash];

						if(score > 0 && safety < COMBAT_LOST) {
							this->options.push_back(Option(&(*a), d, e->pos_hash, score, safety, PURPOSE_COMBAT));
						}
					}
				}			
			}
		}
	}
}

inline void Bot::addRazeOptions() {
	bug_log << "Bot::addRazeOptions" << endl;
	int score;
	int safety;
	int thash;
	
	for(auto a = state.my_ants.begin(); a != state.my_ants.end(); ++a) {
		if(!a->has_moved) {			
			for(int d = 0; d < NO_OF_DIRS_; d++) {
				thash = state.neighbor_index[a->pos_hash][d];
				if(raze_map[thash] > 0 && defense_map[a->pos_hash] > 0) {
					score = (raze_map[thash] + defense_map[a->pos_hash]) * 100;
					safety = state.combat_status[0][thash];		

					if(score > 0 && safety < COMBAT_LOST) {
						this->options.push_back(Option(&(*a), d, thash, score, safety, PURPOSE_RAZE));
					}				
				}
			}
		}
	}
}

inline void Bot::addWanderlustOptions() {
	double score;
	int thash;
	int safety;
	
	for(auto a = state.my_ants.begin(); a != state.my_ants.end(); ++a) {
		if(!a->has_moved) {
			for(int d = 0; d < NO_OF_DIRS_; d++) {
				thash = state.neighbor_index[a->pos_hash][d];
				auto wmiter = wanderlust_map.values.find(thash);
				if(wmiter != wanderlust_map.values.end()) {
					score = wmiter->second;
					safety = state.combat_status[0][thash];
					
					if(score > 0 && safety < COMBAT_LOST) {
						//add salt(visited_times) to distribute equal orders 
						score = 1.0/score + state.times_visited[thash]*0.001; 
						this->options.push_back(Option(&(*a), d, thash, score, safety, PURPOSE_EXPLORE));
					}
				}
			}			
		}
	}
}

inline void Bot::addExploOptions() {
	int score;
	int thash;
	int safety;
	
	for(auto a = state.my_ants.begin(); a != state.my_ants.end(); ++a) {
		if(!a->has_moved) {
			for(int d = 0; d < NO_OF_DIRS_; d++) {
				thash = state.neighbor_index[a->pos_hash][d];
				score = explore_map[thash];
				safety = state.combat_status[0][thash];
				
				if(score > 0 && safety < COMBAT_LOST) {
					this->options.push_back(Option(&(*a), d, thash, score, safety, PURPOSE_EXPLORE));				
				}
			}			
		}
	}
}

inline void Bot::processMoves() {
	int razers = 0;
	int fighters = 0;
	int max_razers = state.my_ants.size() * RAZER_MAX;
	int max_fighters = state.my_ants.size() * FIGHTER_MAX;
	
	for(auto o = options.begin(); o != options.end(); o++) {
		if(o->purpose == PURPOSE_RAZE) {
			if(razers >= max_razers) {
				continue;
			}
		}
		
		else if(o->purpose == PURPOSE_COMBAT) {
			if(fighters >= max_fighters) {
				continue;
			}
		}
		
		if(o->purpose == PURPOSE_COLLECT && ignore_locs[o->target_hash] >= 1) {
			continue;
		}
		if(o->purpose == PURPOSE_COMBAT && ignore_locs[o->target_hash] >= 2) {
			continue;
		}
		if(o->purpose == PURPOSE_RAZE && ignore_locs[o->target_hash] >= max_razers * 0.5) {
			continue;
		}
		
		
		if(!o->ant->has_moved) {

			int thash = state.neighbor_index[o->ant->pos_hash][o->dir];
			if(state.isPassable(thash)) {					

				ignore_locs[o->target_hash]++;
				
				if(o->purpose == PURPOSE_RAZE) {
					razers++;
				} 
				
				
				else if(o->purpose == PURPOSE_COMBAT) {
					fighters++;
				}					
				
				//move
				state.sendMove(*(o->ant), o->dir);
				swap(state.ant_map[o->ant->pos_hash], state.ant_map[thash]);
				o->ant->has_moved = true;
			} else if(o->dir == 4) {
				o->ant->has_moved = true;
			}

		}
	}
}

//plays a single game of Ants.
inline void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    setup();
	endTurn();

    //continues making moves while the game is not over
    while (cin >> state) {
		bug_log << "### TURN: " << GameData::turn << " ###" << endl;
		state.update();
		precalculate();
		
		//breakStalemates();
		
		if(GameData::turn == 1) {
			for(auto hiter = state.my_hills.begin(); hiter != state.my_hills.end(); ++hiter) {
				wanderlust_map.insertHillAt(*hiter);
			}
			wanderlust_map.init(state);	
		} else {
			wanderlust_map.update(state);
		}		
		
		wanderlust_map.updateDeadTiles(state);
		
		//wanderlust_map.paintToVisualizer();
		
		//defense!
/*		
		if(state.my_hills.size() == 1) {
			options.clear();
			addDefenseOptions();
			sort(options.begin(), options.end());
			processMoves();
		}
*/		
		//process combat
		options.clear();
		if(state.my_hills.size() > 0) {
			addCombatOptions();
		}
		//sort(options.begin(), options.end());
		//processMoves();
		
		//add raze options
		//options.clear();
		addRazeOptions();		
		//sort(options.begin(), options.end());
		//processMoves();
		
		//add food options
		//options.clear();
		if(state.my_hills.size() > 0) {
			addFoodOptions();			
		}
		sort(options.begin(), options.end());
		processMoves();

		//exploration via wanderlust		
		options.clear();
		addWanderlustOptions();
		sort(options.begin(), options.end());
		processMoves();
		
		
		//exploration via stumbling
		options.clear();
		//addStumbleOptions();
		addExploOptions();
		sort(options.begin(), options.end());
		processMoves();
		
		
		
        cleanUp();
        endTurn();
    }
}

inline void Bot::cleanUp()
{
	options.clear();	
	fill(ignore_locs.begin(), ignore_locs.end(), 0);
	food_distmaps.clear();
}
 
//finishes the turn
inline void Bot::endTurn()
{
    if (GameData::turn > 0) {
        state.afterTurn();		
	}
    GameData::turn++;	
	bug_log << "TURN -- TIME ELAPSED: " << ttt.getTime() << endl << endl;
    cout << "go" << endl;	
}

#endif //BOT_H_
