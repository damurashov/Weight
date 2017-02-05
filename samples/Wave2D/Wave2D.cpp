#include "MASS.h"
#include "Wave2DMassPlace.h"
#include <stdlib.h>
#include <vector>
#include "Timer.h"


#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

#define PLACES_HANDLE 1

Timer timer;

int main(int argc, char* argv[]) {
	if (argc != 10) {
		std::cerr << "usage: ./main username password machinefile port nProc nThr xSize ySize totalTurns" << std::endl;
		exit(EXIT_FAILURE);
	}

	// prep command line args
	char* arguments[4];
	arguments[0] = argv[1]; 				// username
	arguments[1] = argv[2]; 				// password
	arguments[2] = argv[3]; 				// machinefile
	arguments[3] = argv[4]; 				// port
	int nProcesses = atoi(argv[5]); 		// # of processes
	int nThreads = atoi(argv[6]);			// # of threads
	int xSize = atoi(argv[7]); 				// # size of x side of square matrix
	int ySize = atoi(argv[8]); 				// # size of y side of square matrix
	int totalTurns = atoi(argv[9]); 		// total simulation turns

	// Initialize MASS library
	if (printOutput) std::cerr << "About to initialize MASS Library" << std::endl;
	MASS::init(arguments, nProcesses, nThreads);

	// Initialize Places
	int boundary_width = 2;
	Places* wave2dPlaces = new Places(PLACES_HANDLE, "Wave2DMassPlace", boundary_width, (void*) NULL, 0, 2, xSize, ySize);
	wave2dPlaces->callAll(Wave2DMassPlace::init_);
	wave2dPlaces->exchangeBoundary();
	
	// compute first wave
	wave2dPlaces->callAll(Wave2DMassPlace::computeFirstTurn_);

	// Run Simulation
	for (int currentTurn = 0; currentTurn < totalTurns; currentTurn++) {
		if (printOutput) std::cerr << "Inside simulation loop for turn " << currentTurn << std::endl;
		
		// load data to share into outMessage then exchange with remote spaces
		wave2dPlaces->callAll(Wave2DMassPlace::loadOutMessage_);
		wave2dPlaces->exchangeBoundary();
		
		// compute the next wave at each position
		wave2dPlaces->callAll(Wave2DMassPlace::computeWave_);
		
		if (printOutput) std::cerr << "Finished simulation loop for turn " << currentTurn << std::endl;
	}

	// Get Simulation Output
	double* output = (double*)wave2dPlaces->callAll(Wave2DMassPlace::collectData_, NULL, 0, sizeof(double));

	// Display Simulation Output
	for (int y = 0; y < ySize; y++) {
		for(int x = 0; x < xSize; x++) {
			std::cout << *(output + (x * ySize) + y) << " ";
		}
		std::cout << std::endl;
	}

	// Cleanup MASS library remote processes
	MASS::finish();

	std::cerr << "Total simulation time (usec) for " << xSize << "x" << ySize << " with " << totalTurns << " turns: " << timer.getUsec() << std::endl;
}
