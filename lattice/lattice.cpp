/*
 * lattice.cpp
 *
 * TODO This is very initial prototype code that needs to be refactored heavily.
 *
 *  Created on: 27.4.2020
 *      Author: magi
 */

#include "population.h"

#include <iostream>
#include <fstream>
#include <random>
#include <string.h>
#include <chrono>

default_random_engine generator;
uniform_real_distribution<double> uniformDistribution0to1(0.0,1.0);

enum NeighbourhoodType {
	UNIFORM_NEIGHBOURHOOD = 0,
	SMALL_NEIGHBOURHOOD   = 1,
};


struct ModelParameters {
	int generations = 200;
	int rows = 220;
	int cols = 250;
	double R0 = 2.4;
	NeighbourhoodType neighbourhood = UNIFORM_NEIGHBOURHOOD;
	double smallNeighbourhoodSuppression = 0.0;
	string animationFilename = "animation.gif";
	string reportFilename = "test.csv";

	ModelParameters(int argc, char **argv) {
		for (int i=1; i<argc; i++) {
			string arg = argv[i];
			int eqPos = arg.find('=', 0);
			if (eqPos >= 0) {
				string keyword, value;
				try {
					keyword = arg.substr(0, eqPos);
					value = arg.substr(eqPos+1, string::npos);
				} catch (exception& e) {
					cerr << "Invalid parameter " << arg << endl;
					throw e;
				}

				if (keyword == "generations")
					generations = stoi(value);
				else if (keyword == "R0")
					R0 = stod(value);
				else if (keyword == "neighbourhood")
					neighbourhood = (value == "uniform")? UNIFORM_NEIGHBOURHOOD : SMALL_NEIGHBOURHOOD;
				else if (keyword == "suppression")
					smallNeighbourhoodSuppression = stod(value);
				else if (keyword == "animation")
					animationFilename = value;
				else if (keyword == "report")
					reportFilename = value;
				else {
					cerr << "Unknown parameter '" << keyword << "'\n";
					throw invalid_argument("Unknown parameter");
				}
			}
		}
	}
};

int main(int argc,char **argv) {
	cout << "Starting...\n";

	ModelParameters parameters(argc, argv);

	// Initialize graphics output
	InitializeMagick(*argv);
	list<Image> frames;
	Geometry imageSize(parameters.cols, parameters.rows);

	generator.seed(std::chrono::system_clock::now().time_since_epoch().count());

	// We use double-buffering with population generations
	LatticePopulation previous(parameters.rows, parameters.cols);
	LatticePopulation current(parameters.rows, parameters.cols);

	// Inject infection in the initial population
	previous.getPerson(parameters.cols/2, parameters.rows/2).setState(INFECTED);
	cout << "Initial infected =" << (previous.getPerson(parameters.cols/2, parameters.rows/2).getState()==INFECTED) << "\n";

#ifdef ODE
	// Hack to make compatible with the ODE model start parameters with 0.001 initial infected
	for (int i=0; i<55; i++)
		previous.getPerson(parameters.cols*uniformDistribution0to1(generator), parameters.rows*uniformDistribution0to1(generator)).setState(INFECTED);
#endif

	// Initial image
	Image image(imageSize, ColorMono(false));
	previous.visualize(image);
	image.animationDelay(10);
	frames.push_back(image); // Copies the image

	ofstream outs;
	outs.open (parameters.reportFilename, ofstream::out | ofstream::trunc);

	// Copy the states
	current = previous;

	// For calculating moving average of how many others each
	// infected had infected on average.
	list<int> lastRecoveredInfected;

	// Test
	for (int i=0; i<14; i++)
		cout << "Pow test " << ((2.4/3.25)/(1+pow(0.84, double(i)))) << endl;

	cout << "Starting simulation...\n";
	for (int i=0; i<parameters.generations + 1; i++) {
		cout << "Generation " << i << "\n";

		// Iterate through the population
		for (int row=0; row < parameters.rows; row++)
			for (int col=0; col < parameters.cols; col++) {
				Person& person = previous.getPerson(col, row);
				Person& currentPerson = current.getPerson(col, row);

				// Infect others
				if (person.getState() == INFECTED) {
					// Infection rate drops by time
					// TODO This probably has quite different distribution than what ODE models assume, so it produces different rate.
					// This sums to R0 over 14 days with average at 3 days
					// Python code to estimate it:
					// ep=0.86; a=sum(map(lambda x: (1/1.00)/(1.0+x**ep),range(0,3))); b=sum(map(lambda x: (1/1.00)/(1.0+x**ep),range(0,15)));a/b,a,b
					double infRate = (parameters.R0 / 3.7)/(1.0 + pow(double(person.getDaysInState()), 0.86));

					double number = uniformDistribution0to1(generator);
					if (number <= infRate) {
						// Infect someone in the neighbourhood
						Position infectedPos;
						switch (parameters.neighbourhood) {
						case UNIFORM_NEIGHBOURHOOD:
							infectedPos = current.uniformNeighbourhood(Position(col, row));
							break;
						case SMALL_NEIGHBOURHOOD:
							infectedPos = current.smallNeighbourhood(Position(col, row), parameters.smallNeighbourhoodSuppression);
							break;
						}

						Person& personToInfect = current.getPerson(infectedPos);
						if (personToInfect.getState() == SUSCEPTIBLE) {
							personToInfect.setState(EXPOSED);

							// For counting current R
							currentPerson.incrementInfectedCount();
						}
					}

					if (currentPerson.getDaysInState() >= 14) {
						currentPerson.setState(RECOVERED);

						// We count average of how many each recovered infected during infection.
						// This is to get an estimate of the current R.
						lastRecoveredInfected.push_back(currentPerson.getInfectedCount());
						if (lastRecoveredInfected.size() > 20)
							lastRecoveredInfected.pop_front();
					}
				}

				// Becomes infected after 3 days (2 discounting today)
				if (person.getState() == EXPOSED && currentPerson.getDaysInState() >= 2)
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
		writeImages(frames.begin(), frames.end(), parameters.animationFilename);
	} catch (Exception& error_) {
	      cout << "Failed to write images: " << error_.what() << endl;
	      return 1;
	}
	return 0;
}
