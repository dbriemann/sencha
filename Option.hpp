
#include "Ant.hpp"

struct Option {
	Ant *ant;
	int dir;
	int target_hash;
	double distance;
	int safety;
	int purpose;
	
	Option(Ant *ant, const int &dir, const int &thash, const double &dist, const int &safety, const int &purpose);
	
	bool operator<(const Option& right) const;
};

inline Option::Option(Ant *ant, const int &dir, const int &thash, const double & dist, const int &safety, const int &purpose) {
	this->ant = ant;
	this->dir = dir;
	this->target_hash = thash;
	this->distance = dist;
	this->purpose = purpose;
	this->safety = safety;
}

inline bool Option::operator <(const Option &right) const {
	if(this->safety < right.safety) {
		return true;
	} else if(this->safety == right.safety) {
		return (this->distance < right.distance);
	} else {
		return false;
	}    
}

