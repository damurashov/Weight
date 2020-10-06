//
// Created by sarah on 4/8/20.
//

#include "Test.h"

extern "C" Place *instantiate(void *argument) {
    return new Test(argument);
}

extern "C" void destroy(Place *object) {
    delete object;
}

void *Test::init(void *showPlaceId) {
    bool print = *(bool *) (showPlaceId);
    if (print) {
        convert.str("");
        convert << "Hello World! I'm TestPlace " << index[0] << " , " << index[1] << " ,and I have " << agents.size() <<
        " agents and " << getNeighbors().size() << " neighboring TestPlaces." << endl;
        MASS_base::log(convert.str());
    }
    return nullptr;
}

void *Test::testingAgentsOnPlace(void *print) {
    bool printOut = *(bool *) (print);
    if (printOut) {
        convert.str("");
        convert << "Testing Place's Agent Count : Place " << index[0] << " , " << index[1] << " has " << agents.size()
                << " agents " << endl;
        MASS_base::log(convert.str());
    }
    return nullptr;
}
