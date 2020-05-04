/*
 * population.cpp
 *
 *  Created on: 4.5.2020
 *      Author: magi
 */

#include "population.h"

#include <random>
#include <string.h>

ColorRGB stateColors[] = {
		ColorRGB(1,1,1),
		ColorRGB(1,1,0),
		ColorRGB(1,0,0),
		ColorRGB(0.5,0.5,1), // RECOVERED
		ColorRGB(1,0.5,0.5), // HOSPITALIZED
		ColorRGB(0.75,0.75,0.75),
		ColorRGB(0.0,0.0,0.0),
};

void LatticePopulation::operator=(const LatticePopulation& other) {
	memcpy(persons, other.persons, sizeof(Person)*rows*cols);
}

void LatticePopulation::visualize(Image& image)
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

static default_random_engine generator;
static uniform_real_distribution<double> uniformDistribution0to1(0.0,1.0);
static bernoulli_distribution bernoulli;

Position LatticePopulation::uniformNeighbourhood(const Position& pos)
{
	// Here self-infection is so unlikely that we don't bother to eliminate it
	int infRow = int(uniformDistribution0to1(generator)*rows);
	int infCol = int(uniformDistribution0to1(generator)*cols);
	return Position(infCol, infRow);
}

Position LatticePopulation::smallNeighbourhood(const Position& pos, double suppression)
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
