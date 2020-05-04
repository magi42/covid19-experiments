/*
 * lattice.cpp
 *
 * TODO This is very initial prototype code that needs to be refactored heavily.
 *
 *  Created on: 27.4.2020
 *      Author: magi
 */

#include <iostream>
#include <fstream>
#include <random>
#include <Magick++.h>
#include <string.h>

using namespace std;
using namespace Magick;

enum ModelState {
	SUSCEPTIBLE  = 0,
	EXPOSED      = 1,
	INFECTED     = 2,
	RECOVERED    = 3,
	HOSPITALIZED = 4,
	ICU          = 5,
	DECEASED     = 6,
};

ColorRGB stateColors[] = {
	ColorRGB(1,1,1),
	ColorRGB(1,1,0),
	ColorRGB(1,0,0),
	ColorRGB(0.5,0.5,1), // RECOVERED
	ColorRGB(1,0.5,0.5), // HOSPITALIZED
	ColorRGB(0.75,0.75,0.75),
	ColorRGB(0.0,0.0,0.0),
};

class Person {
	ModelState state = SUSCEPTIBLE;
	int ageGroup = 0;
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

class Position {
	int col;
	int row;
public:
	Position(int col, int row) : col(col), row(row) {}
	int getCol() const {return col;}
	int getRow() const {return row;}
};

class PersonLattice {
private:
	int rows;
	int cols;
	Person* persons = nullptr;

public:
	PersonLattice(int rows, int columns)
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

	void operator=(const PersonLattice& other) {
		memcpy(persons, other.persons, sizeof(Person)*rows*cols);
	}

	void visualize(Image& image);

	struct Demographics {
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

void PersonLattice::visualize(Image& image)
{
	image.modifyImage();
	image.type(TrueColorType);
	PixelPacket* pixels = image.getPixels(0, 0, cols, rows);

	for (int row=0; row<rows; row++)
		for (int col=0; col<cols; col++) {
			Person& person = getPerson(col, row);
			pixels[row*cols+col] = stateColors[person.getState()];
		}

	image.syncPixels();
}

default_random_engine generator;
uniform_real_distribution<double> uniformDistribution0to1(0.0,1.0);
bernoulli_distribution bernoulli;

Position PersonLattice::uniformNeighbourhood(const Position& pos)
{
	// Here self-infection is so unlikely that we don't bother to eliminate it
	int infRow = int(uniformDistribution0to1(generator)*rows);
	int infCol = int(uniformDistribution0to1(generator)*cols);
	return Position(infCol, infRow);
}

Position PersonLattice::smallNeighbourhood(const Position& pos, double suppression)
{
	int infRow, infCol;
	do {
		int rowOffset = (1/(suppression+uniformDistribution0to1(generator)*(1-suppression)) - 1) * (bernoulli(generator)? 1:-1);
		int colOffset = (1/(suppression+uniformDistribution0to1(generator)*(1-suppression)) - 1) * (bernoulli(generator)? 1:-1);

		infCol = (((pos.getCol() + colOffset) % cols) + cols) % cols;
		infRow = (((pos.getRow() + rowOffset) % rows) + rows) % rows;

		// We loop until we don't try to infect self
	} while (infCol == pos.getCol() && infRow == pos.getRow());
	return Position(infCol, infRow);
}

int main(int argc,char **argv) {
	cout << "Starting...\n";

	// How long to run
	const int generations = 1000;

	// Population size
	const int rows = 220;
	const int cols = 250;

	// Parameters
	const double R0=2.4;

	// Initialize graphics output
	InitializeMagick(*argv);
	list<Image> frames;
	Geometry imageSize(cols, rows);

	// We use "double-buffering"
	PersonLattice previous(rows,cols);
	PersonLattice current(rows,cols);

	// Inject infection in the initial population
	previous.getPerson(cols/2, rows/2).setState(INFECTED);
	cout << "Initial infected =" << (previous.getPerson(cols/2, rows/2).getState()==INFECTED) << "\n";

	// Initial image
	Image image(imageSize, ColorMono(false));
	previous.visualize(image);
	image.animationDelay(10);
	frames.push_back(image); // Copies the image
	image.write("initial.gif");

	ofstream outs;
	outs.open ("log.csv", ofstream::out | ofstream::trunc);

	// Copy the states
	current = previous;

	// For calculating moving average of how many others each
	// infected had infected on average.
	list<int> lastRecoveredInfected;

	cout << "Starting simulation...\n";
	for (int i=0; i<generations + 1; i++) {
		cout << "Generation " << i << "\n";

		// Iterate through the population
		for (int row=0; row<rows; row++)
			for (int col=0; col<cols; col++) {
				Person& person = previous.getPerson(col, row);
				Person& currentPerson = current.getPerson(col, row);

				// Infect others
				if (person.getState() == INFECTED) {
					// Infection rate drops by time
					// This sums to R0 over 14 days
					// TODO Better function. This one has average at about 4 days, not 3.
					double infRate = (R0/3.25)/(1.0 + person.getDaysInState());

					double number = uniformDistribution0to1(generator);
					if (number <= infRate) {
						// Infect someone in the neighbourhood
						// Position infectedPos = current.uniformNeighbourhood(Position(col, row));
						Position infectedPos = current.smallNeighbourhood(Position(col, row), 0.2);

						Person& personToInfect = current.getPerson(infectedPos);
						if (personToInfect.getState() == SUSCEPTIBLE) {
							personToInfect.setState(EXPOSED);

							// For counting current R
							currentPerson.incrementInfectedCount();
						}
					}

					if (currentPerson.getDaysInState() > 14) {
						currentPerson.setState(RECOVERED);

						// We count average of how many each recovered infected during infection.
						// This is to get an estimate of the current R.
						lastRecoveredInfected.push_back(currentPerson.getInfectedCount());
						if (lastRecoveredInfected.size() > 20)
							lastRecoveredInfected.pop_front();
					}
				}

				if (person.getState() == EXPOSED && currentPerson.getDaysInState() >= 3)
					currentPerson.setState(INFECTED);

				// Don't increment days if the person just had a state change
				if (person.getState() == currentPerson.getState())
					currentPerson.incrementDaysInState();
			}
		previous = current;

		// Reporting for the generation
		outs << i << " ";
		current.writeStatistics(outs);

		// Calculate moving average of how many the latest recovered infected
		// to obtain an estimate of the current R.
		int recovered = lastRecoveredInfected.size();
		int recoveredInfected = 0;
		for (int x: lastRecoveredInfected)
			recoveredInfected += x;
		double Restimate = ((recovered>0)?(double(recoveredInfected)/double(recovered)):0);
		outs << " " << Restimate;
		outs << endl;

		// Visualize the current generation
		Image image(imageSize, ColorMono(false));
		current.visualize(image);
		image.animationDelay(10);
		frames.push_back(image); // Copies the image
	}

	outs.close();
	cout << "Simulation ended, writing animation...\n";

	try {
		writeImages(frames.begin(), frames.end(), "animation.gif");
	} catch (Exception& error_) {
	      cout << "Failed to write images: " << error_.what() << endl;
	      return 1;
	}
	return 0;
}
