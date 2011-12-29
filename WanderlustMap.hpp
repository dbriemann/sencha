
#include <unordered_map>
#include <queue>
using namespace std;

struct WanderlustMap {
	unordered_map<int,bool> dead_color_tiles;
	
	unordered_map<int,int> values;
	queue<int> border_tiles;
	queue<int> dead_tiles;
	
	WanderlustMap();
	
	void print(const State &state);
	void paintToVisualizer();
	
	void insertHillAt(const int &hash);
	int findSmallestNeighbor(const State &state, const int &hash);
	void init(const State &state);
	void update(const State &state);
	void updateDeadTiles(const State &state);
	void setDeadTiles(const State &state);
	bool isADeadEnd(const State &state, const int &hash);
	void propagate(const State &state, queue<int> &nodes);
};

inline WanderlustMap::WanderlustMap() {
	values = unordered_map<int,int>();
	border_tiles = queue<int>();
	dead_tiles = queue<int>();
}

inline void WanderlustMap::insertHillAt(const int &hash) {
	values[hash] = 0;
}

inline int WanderlustMap::findSmallestNeighbor(const State &state, const int &hash) {
	int smallest = INT_MAX;
	int thash;	
	
	for(int d = 0; d < NO_OF_DIRS; d++) {
		thash = state.neighbor_index[hash][d];
		
		auto viter = values.find(thash);
		if( viter != values.end() && viter->second >= 0 && viter->second < smallest) {
			smallest = viter->second;
		}
	}
	return smallest;
}

inline void WanderlustMap::print(const State &state) {
	for(int r = 0; r < GameData::rows; r++) {
		for(int c = 0; c < GameData::cols; c++) {
			int hash = locationToHash(Location(r,c));
			
			auto viter = values.find(hash);
			if(state.water[hash] == 1) {
				bug_log << '~';
			}else if(viter == values.end()) {
				bug_log << '?';
			} else {
				if(viter->second >= 0 && viter->second < 10) {
					bug_log << viter->second;
				} else if(viter->second >= 10) {
					bug_log << '+';
				} else {
					bug_log << '-';				
				}
			}
		}
		bug_log << endl;
	}
	bug_log << endl;
}

inline void WanderlustMap::propagate(const State &state, queue<int> &nodes) {	
	unordered_map<int, bool> visited = unordered_map<int, bool>();
	int nodehash, thash, smallest;
	
	while(!nodes.empty()) {
		nodehash = nodes.front(); nodes.pop();
		
		if(!visited[nodehash]) {			
			for(int d = 0; d < NO_OF_DIRS; d++) {
				thash = state.neighbor_index[nodehash][d];
				
				//not water and no value yet:
				if(!state.isWaterAt(thash) && (values.find(thash) == values.end())) {
					smallest = findSmallestNeighbor(state, thash);
					
					if(state.isExplored(thash) && smallest < INT_MAX) {	
						values[thash] = smallest + 1;
													
						nodes.push(thash);
					} else {
						border_tiles.push(nodehash);
					}
				}
			}
			
			
			auto valiter = values.find(nodehash);
			if(valiter != values.end()) {
				smallest = findSmallestNeighbor(state, nodehash);
				
				if(smallest + 1 < values[nodehash]) {
					//a connection between two joining paths 
					//reassign values
					values[nodehash] = smallest + 1;
					
					//reinsert alle neighbors
					for(int d = 0; d < NO_OF_DIRS; d++) {
						thash = state.neighbor_index[nodehash][d];
						
						if(!state.isWaterAt(thash) && state.isExplored(thash)) {
							visited[thash] = false;
							nodes.push(thash);
						}
					}
				}
			}
		}
		
		visited[nodehash] = true;
	}	
}

inline void WanderlustMap::setDeadTiles(const State &state) {
	for(auto viter = values.begin(); viter != values.end(); ++viter) {
		if(isADeadEnd(state, viter->first)) {
			dead_tiles.push(viter->first);
		}
	}
}

inline bool WanderlustMap::isADeadEnd(const State &state, const int &hash) {
	int curval = values[hash];
	int thash;
	bool result = true;
		
	
	if(state.enemy_hills.find(hash) != state.enemy_hills.end()) {
		return false;
	}
	
	for(int d = 0; d < NO_OF_DIRS; d++) {
		thash = state.neighbor_index[hash][d];
		auto viter = values.find(thash);
		if(viter != values.end()) {
			if(viter->second > curval) {
				result = false;
				break;
			}
		} else if(!state.isWaterAt(thash)) {
			result = false;
			break;
		}
	}
	
	if(result) {
		dead_color_tiles[hash] = true;
	}
	
	return result;
}

inline void WanderlustMap::paintToVisualizer() {
	Location lol;
	cout << "v setFillColor 255 0 0 0.5" << endl;
	
	for(auto dciter = dead_color_tiles.begin(); dciter != dead_color_tiles.end(); ++dciter) {
		if(dciter->second) {
			lol = hashToLocation(dciter->first);
			cout << "v tile " << lol.row << " " << lol.col << endl;
		}
	}	
}

inline void WanderlustMap::updateDeadTiles(const State &state) {
	setDeadTiles(state);
	int nodehash, thash;
	unordered_map<int, bool> visited = unordered_map<int,bool>();
	
	while(!dead_tiles.empty()) {
		nodehash = dead_tiles.front(); dead_tiles.pop();		
		
		if(!visited[nodehash] && isADeadEnd(state, nodehash)) {
			values[nodehash] = -values[nodehash]; //flip values
			
			//put all neighbors in dead queue
			for(int d = 0; d < NO_OF_DIRS; d++) {
				thash = state.neighbor_index[nodehash][d];
				
				if(!visited[thash] && !state.isWaterAt(thash) && values.find(thash) != values.end()) {
					dead_tiles.push(thash);
				}
			}
		}
		
		visited[nodehash] = true;
	}
}

inline void WanderlustMap::update(const State &state) {
	queue<int> nodes;
	
	while(!border_tiles.empty()) {
		nodes.push(border_tiles.front());
		border_tiles.pop();		
	}

	propagate(state, nodes);
}

inline void WanderlustMap::init(const State &state) {
	queue<int> nodes;	
	
	for(auto siter = values.begin(); siter != values.end(); ++siter) {
		//add hills as origins
		nodes.push(siter->first);
	}
	
	propagate(state, nodes);
}

