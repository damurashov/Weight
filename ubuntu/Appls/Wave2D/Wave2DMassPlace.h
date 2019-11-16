#ifndef WAVE_2D_MASS_PLACE_H
#define WAVE_2D_MASS_PLACE_H

#include <string.h>
#include <sstream>
#include "Place.h"

class Wave2DMassPlace : public Place {
public:
	static int north[];
	static int east[];
	static int south[];
	static int west[];

	// define functionId's
	static constexpr int init_ = 0;
	static constexpr int computeFirstTurn_ = 1;
	static constexpr int computeWave_ = 2;
	static constexpr int loadOutMessage_ = 3;
	static constexpr int collectData_ = 4;
	
	// constructor
	Wave2DMassPlace(void* argument);

	// entry point for callAll()
	virtual void* callMethod(int functionId, void* argument) {
		switch(functionId) {
			case init_:
				init();
				break;
			case computeFirstTurn_: 
				computeFirstTurn();
				break;
			case computeWave_: 
				computeWave();
				break;
			case loadOutMessage_:
				loadOutMessage();
				break;
			case collectData_: return collectData();
		}
		return NULL;
	};

private:
	// methods to be called by callAll()
	void init();
	void computeFirstTurn();
	void computeWave();
	void loadOutMessage();
	void* collectData();

	// for easy state logging
	void logStateWithMessage(std::string message);

	double getOtherWaveState(int* neighbor);

	// private state data
	double currentWave;
	double previousWave;
	int sizeX;
	int sizeY;
	int myX;
	int myY;

	// for logging
	std::ostringstream convert;

	// these may become a constructor param
	static constexpr double STARTING_WAVE = 20.0;
	static constexpr double c = 1.0;
	static constexpr double dt = 0.1;
	static constexpr double dd = 2.0;
};

#endif

