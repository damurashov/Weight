//
// 
//

#include "TestPlace.h"

extern "C" Place *instantiate_from_file(string filename, FILE_TYPE_ENUMS type, 
                                            int global, void *arg, std::unordered_map<string, int> *dist_map) {
    return new TestPlace(filename, type, global, arg, dist_map);
}


extern "C" Place *instantiate(void *argument) {
    return new TestPlace(argument);
}

extern "C" void destroy(Place *object) {
    delete object;
}

void *TestPlace::init(void *showPlaceId) {
    bool print = *(bool *) (showPlaceId);
    if (print) {
        convert.str("");
        convert << "Hello World! I'm TestPlace " << index[0] << " , " << index[1] << " ,and I have " << agents.size() <<
        " agents and " << getNeighbors().size() << " neighboring TestPlaces." << endl;
        MASS_base::log(convert.str());
    }
    return nullptr;
}

void *TestPlace::testingAgentsOnPlace(void *print) {
    bool printOut = *(bool *) (print);
    if (printOut) {
        convert.str("");
        convert << "Testing Place's Agent Count : Place " << index[0] << " , " << index[1] << " has " << agents.size()
                << " agents " << endl;
        MASS_base::log(convert.str());
    }
    return nullptr;
}
