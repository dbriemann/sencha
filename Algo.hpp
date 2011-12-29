
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;

#include "State.hpp"

//map vector will be cleared
static inline void genDistMap(const int &source, vector<int> &dmap, const int &maxdist, const State &state, const int &ant_malus) {
	//reset data
    fill(dmap.begin(), dmap.end(), -1);
	
	queue<int> nodes = queue<int>();

	Location node_loc, target_loc;
	int target_hash, node_hash;
	
	//init queue
	nodes.push(source);
	dmap[source] = 1; //set roots to 1 (avoid divisions by zero in eval)	
	
	while(!nodes.empty()) {
		node_hash = nodes.front(); nodes.pop();
		
		for(int d=0; d < NO_OF_DIRS; d++) {
			target_hash = state.neighbor_index[node_hash][d];

			if(dmap[target_hash] < 0 && (dmap[node_hash] < maxdist) && !state.isWaterAt(target_hash)) {
				nodes.push(target_hash);

				if(state.ant_map[target_hash] != NULL && state.ant_map[target_hash]->owner == 0) {
					dmap[target_hash] = dmap[node_hash] + ant_malus;
				} else {
					dmap[target_hash] = dmap[node_hash] + 1;
				}
			}
		}
	}	
}

//map vector will be cleared
static inline void genMultiDistMap(const vector<int> &sources, vector<int> &dmap, const int &maxdist, const State &state, const int &ant_malus) {
	//reset data
    fill(dmap.begin(), dmap.end(), -1);
	
	queue<int> nodes = queue<int>();

	Location node_loc, target_loc;
	int target_hash, node_hash;
	
	//init queue
	for(int h = 0; h < GameData::rows * GameData::cols; h++) {
		if(sources[h] == 1) {
			nodes.push(h);
			dmap[h] = 1; //set roots to 1 (avoid divisions by zero in eval)			
		}
	}
	
	while(!nodes.empty()) {
		node_hash = nodes.front(); nodes.pop();
		
		for(int d=0; d < NO_OF_DIRS; d++) {
			target_hash = state.neighbor_index[node_hash][d];
			//not visited && ( < maxdist && no water)
			if(dmap[target_hash] < 0 && (dmap[node_hash] < maxdist) && !state.isWaterAt(target_hash)) {
				nodes.push(target_hash);
				//TEST: make paths longer which contain own ants

				if(state.ant_map[target_hash] != NULL && state.ant_map[target_hash]->owner == 0) {
					dmap[target_hash] = dmap[node_hash] + ant_malus;
				} else {
					dmap[target_hash] = dmap[node_hash] + 1;
				}
			}
		}
	}	
}

static inline void printDistmap(vector<int> &dm)
{
    int hash;
    Location loc;
    bug_log << "DistMap" << endl;

    for(int r = 0; r < GameData::rows; r++) {
        for(int c = 0; c < GameData::cols; c++) {
            loc = getModuloLocation(r,c);
            hash = locationToHash(loc);
			if(dm[hash] == -1) {
				bug_log << "X";
			} else if(dm[hash] <= 9) {
				bug_log << dm[hash] ;
			} else {
                bug_log << 'B';
            }
        }
        bug_log << endl;
    }
    bug_log << endl;
}