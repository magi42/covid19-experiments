/*
 * population.h
 *
 *  Created on: 4.5.2020
 *      Author: magi
 */

#ifndef POPULATION_H_
#define POPULATION_H_

#include <Magick++.h>

using namespace std;
using namespace Magick;

enum ModelState {
	SUSCEPTIBLE  = 0,
	EXPOSED      = 1,
	INFECTED     = 2,
	RECOVERED    = 3,
	HOSPITALIZED = 4, // TODO Currently unused
	ICU          = 5, // TODO Currently unused
	DECEASED     = 6, // TODO Currently unused
};

extern ColorRGB stateColors[];

class Person {
	ModelState state = SUSCEPTIBLE;
	int ageGroup = 0; // TODO Currently unused
	int daysInState = 0;
	int infectedCount = 0;

public:
	ModelState getState() const {return state;}
	void setState(ModelState newState) {state = newState; daysInState=0;}

	int getDaysInState() const {return daysInState;}
	void incrementDaysInState() {daysInState++;}

	int getInfectedCount() const {return infectedCount;}
	void incrementInfectedCount() {infectedCount++;}

};

/**
 * Position coordinates on a lattice.
 */
class Position {
	int col;
	int row;
public:
	Position() = default;
	Position(int col, int row) : col(col), row(row) {}
	int getCol() const {return col;}
	int getRow() const {return row;}
};

class LatticePopulation {
private:
	int rows;
	int cols;
	Person* persons = nullptr;

public:
	LatticePopulation(int rows, int columns)
		: rows(rows), cols(columns)
	{
		persons = new Person[rows*columns];

		// Initialize the population
		for (int row=0; row<rows; row++)
			for (int col=0; col<columns; col++) {
				Person& person = getPerson(col, row);
				person.setState(SUSCEPTIBLE);
			}

	}

	Person& getPerson(int col, int row)
	{
		return persons[row*cols+col];
	}

	Person& getPerson(const Position& pos)
	{
		return persons[pos.getRow()*cols+pos.getCol()];
	}

	void operator=(const LatticePopulation& other);

	/**
	 * Visualizes the current population to the given image.
	 */
	void visualize(Image& image);

	// TODO This is unused at the moment
	struct Statistics {
		int susceptible = 0;
		int exposed     = 0;
		int infected    = 0;
		int recovered   = 0;
		int hospitalized= 0;
		int icu         = 0;
		int deceased    = 0;
	};

	void writeStatistics(ostream& out) {
		vector<int> stateCounts(7);
		for (int& pos: stateCounts)
			pos = 0;

		for (int row=0; row<rows; row++)
			for (int col=0; col<cols; col++) {
				Person& person = getPerson(col, row);
				stateCounts[person.getState()]++;
			}

		for (int i=0; i<stateCounts.size(); i++) {
			if (i>0)
				out << " ";
			out << stateCounts[i];
		}
	}

	Position uniformNeighbourhood(const Position& pos);
	Position smallNeighbourhood(const Position& pos, double suppression);
};



#endif /* POPULATION_H_ */
