/*
 * utils.hpp
 *
 *  Created on: Oct 27, 2011
 *      Author: dlb
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cmath>
#include <map>
using namespace std;

#include "Location.hpp"
#include "Definitions.hpp"
#include "GameData.hpp"


static inline int locationToHash(Location loc)
{
    return loc.row * GameData::cols + loc.col;
}

static inline Location hashToLocation(const int &hash)
{
    int row = hash / GameData::cols;
    int col = hash % GameData::cols;
    Location loc(row, col);
    return loc;
}

//returns the new location from moving in a given direction with the edges wrapped
static inline Location getLocation(const Location &loc, int direction)
{
    return Location((loc.row + AIM[direction][0] + GameData::rows) % GameData::rows,
            (loc.col + AIM[direction][1] + GameData::cols) % GameData::cols);
}

static inline Location getModuloLocation(const int &row, const int &col)
{
    return Location((row + GameData::rows) % GameData::rows, (col  + GameData::cols) % GameData::cols);
}

//returns the euclidean distance between two locations with the edges wrapped
static inline double euclidian(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row - loc2.row);
    int d2 = abs(loc1.col - loc2.col);
    int dr = min(d1, GameData::rows - d1), dc = min(d2, GameData::cols - d2);
    return sqrt(dr * dr + dc * dc);
}

//returns the euclidean distance between two locations with the edges wrapped
static inline int manhattan(const Location &loc1, const Location &loc2) {
    int d1 = abs(loc1.row - loc2.row);
    int d2 = abs(loc1.col - loc2.col);
    int dr = min(d1, GameData::rows - d1);
    int dc = min(d2, GameData::cols - d2);
    return dc + dr;
}

static inline int squareddist(const Location &loc1, const Location &loc2) {
	int d1 = abs(loc1.row - loc2.row);
    int d2 = abs(loc1.col - loc2.col);
    int dr = min(d1, GameData::rows - d1);
    int dc = min(d2, GameData::cols - d2);
    return dc*dc + dr*dr;
}

static inline void genRangeMask(int range2, vector< pair<int,int> > &mask) {
	int range = sqrt(range2)+1;

    for(int c = -range; c <= range; c++) {
        for(int r = -range; r <= range; r++) {
            if((r*r + c*c) <= range2) {
                mask.push_back(make_pair(r,c));
            }
        }
    }
}

static inline void genCombatMask(const int &range2, const int &extraspace, vector< pair<int,int> > &mask) {
	genRangeMask(range2, mask);
	auto visited = map<pair<int,int>, bool>();
	auto dists = map<pair<int,int>, int>();	
	
	//add all pairs to queue
	queue< pair<int,int> > nodes;
	for(auto piter = mask.begin(); piter != mask.end(); ++piter) {
		visited[*piter] = true;
		dists[*piter] = 0;
		nodes.push(*piter);
	}
	
	int nextr,nextc;
	pair<int,int> nextpair;
	
	//add manhattan distance of 2 to the mask
	while(!nodes.empty()) {
		pair<int,int> node = nodes.front(); nodes.pop();
		
		for(int d = 0; d < NO_OF_DIRS; d++) {
			nextr = node.first + AIM[d][0];
			nextc = node.second + AIM[d][1];
			nextpair = make_pair(nextr, nextc);
			if(!visited[nextpair] && dists[node] < extraspace) {
				mask.push_back(nextpair);
				visited[nextpair] = true;
				dists[nextpair] = dists[node] + 1;
				nodes.push(nextpair);
			}
		}
	}
}


#endif /* UTILS_HPP_ */
