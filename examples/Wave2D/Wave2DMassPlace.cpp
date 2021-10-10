#include "Wave2DMassPlace.h"
#include "MASS_base.h"		// for logging
#include <sstream>			// ostringstream

#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

// instantiate and destroy methods required for Place objects
extern "C" Place* instantiate(void* argument) {
	return new Wave2DMassPlace(argument);
}

extern "C" void destroy(Place* object) {
	delete object;
}

// Define relative indexes
int Wave2DMassPlace::north[] = { 0, 1 };
int Wave2DMassPlace::east[] = { 1, 0 };
int Wave2DMassPlace::south[] = { 0, -1 };
int Wave2DMassPlace::west[] = { -1, 0 };

// Default constructor
Wave2DMassPlace::Wave2DMassPlace(void* argument) : Place(argument) {
	if (printOutput) MASS_base::log("Constructed new Wave2DMassPlace");
}

// initialize state, this is not done in the constructor because it relys on
// the state size and index which are not guarenteed to be initialized until
// after the original construction of the Place objects
void Wave2DMassPlace::init() {
	// set messages sizes for exchange boundry
	inMessage_size = sizeof(double);
	outMessage_size = sizeof(double);
	// get total size from underlying Place object
	sizeX = size[0];
	sizeY = size[1];
	// get this objects index from underlying Place obejct
	myX = index[0];
	myY = index[1];
	// set the starting wave, Starting wave in the middle, 0 everywhere else
	previousWave = (sizeX * 0.4 <= myX && myX < sizeX * 0.6 && sizeY * 0.4 <= myY && myY < sizeY * 0.6) ? STARTING_WAVE : 0.0;
	currentWave = 0;
	outMessage = (void*)&currentWave;
	if (printOutput) logStateWithMessage("Finished call to init()");
}

// Factored out state logging for debugging
void Wave2DMassPlace::logStateWithMessage(std::string message) {
	convert.str("");
	convert << message << std::endl;
	convert << "[" << myX << "][" << myY << "] = { currentWave: " << currentWave << ", previousWave: " << previousWave << " }" << std::endl;
	MASS_base::log(convert.str());
}

// Computes the first wave. This is a seperate method because it uses a slightly
// different formula than the general computeWave() method
void Wave2DMassPlace::computeFirstTurn() {
	// First turn is computed with a different formula
	currentWave = previousWave + (c * c / 2.0 * dt * dt / (dd * dd)) * (getOtherWaveState(north) + getOtherWaveState(east) + getOtherWaveState(south) + getOtherWaveState(west) - 4.0 * previousWave);
	if (printOutput) logStateWithMessage("Completed call to computeFirstTurn()");
}

// Computes the current wave at this place.
void Wave2DMassPlace::computeWave() {
	// Compute next wave, then set up outMessage for exchangeBoundary
	double nextWave = 2.0 * currentWave - previousWave + c * c * dt * dt / (dd * dd)
		* (getOtherWaveState(north) + getOtherWaveState(east) + getOtherWaveState(south) + getOtherWaveState(west) - 4.0 * currentWave);
	previousWave = currentWave;
	currentWave = nextWave;
	if (printOutput) logStateWithMessage("Completed call to computeWave()");
}

// Sets the state of outMessage so to be shared between nodes via exchangeBoundary()
void Wave2DMassPlace::loadOutMessage() {
	outMessage = (void*)&currentWave;
}

// Returns the current state of all waves.
void* Wave2DMassPlace::collectData() {
	// Return the data
	double* result = new double;
	*result = currentWave;
	if (printOutput) logStateWithMessage("Completed call to collectData()");
	return (void*) result;
}

// Gets the state of another wave via relative index
double Wave2DMassPlace::getOtherWaveState(int* relativeIndex) {
	// use boundary spaces recieve 0 from sides outside of matrix.
	if (myX + relativeIndex[0] >= sizeX || myX + relativeIndex[0] < 0 ||
			myY + relativeIndex[1] >= sizeY || myY + relativeIndex[1] < 0) {
		return 0.0;
	}
	return *(double*)getOutMessage(1, relativeIndex);
}
