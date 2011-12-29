#ifndef LOCATION_HPP_
#define LOCATION_HPP_

/*
 struct for representing locations in the grid.
 */
struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    }

    Location(int r, int c)
    {
        row = r;
        col = c;
    }
	
	bool operator == (const Location &right) {
		return (this->row == right.row && this->col == right.col);
	}
};

#endif //LOCATION_H_
