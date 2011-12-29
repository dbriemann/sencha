/*
 * Ant.hpp
 *
 *  Created on: Oct 27, 2011
 *      Author: dlb
 */

#ifndef ANT_HPP_
#define ANT_HPP_

#include "Location.hpp"
#include "Utils.hpp"
#include "Definitions.hpp"
#include "Logger.hpp"

struct Ant {
	int pos_hash;
	//int target_hash;
	int target_dir;
	int owner;
	bool has_moved;

    Ant();
    Ant(const Location &loc, const int &owner);
    ~Ant();
};


inline Ant::Ant()
{	
	this->has_moved = false;
	this->target_dir = 4;
}

inline Ant::Ant(const Location &loc, const int &owner)
{
	this->has_moved = false;
	this->target_dir = 4;
	this->pos_hash = locationToHash(loc);
	//this->target_hash = pos_hash;
    this->owner = owner;
}

inline Ant::~Ant()
{
}


#endif /* ANT_HPP_ */
