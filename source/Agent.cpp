/*
 MASS C++ Software License
 © 2014-2015 University of Washington
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 The following acknowledgment shall be used where appropriate in publications,
 presentations, etc.: © 2014-2015 University of Washington. MASS was developed
 by Computing and Software Systems at University of Washington Bothell. THE
 SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Agent.h"
#include <iostream>
#include <sstream>  // ostringstream
#include "MASS_base.h"

/**
 * Spawns a “numAgents’ of new agents, as passing arguments[i] (with arg_size)
 * to the i-th new agent upon a next call to Agents.manageAll( ).
 * More specifically, spawn( ) changes the calling agent’s newChildren.
 * @param numAgents
 * @param arguments
 * @param arg_size
 */
void Agent::spawn(int numAgents, vector<void*> arguments, int arg_size) {
    // Only want to make changes if the number to be created is above zero
    if (numAgents > 0) {
        newChildren = numAgents;
        this->arguments = arguments;
    }
}

int Agent::map(int initPopulation, vector<int> size, vector<int> index,
               Place* curPlace) {
    // compute the total # places
    int placeTotal = 1;
    for (int x = 0; x < int(size.size()); x++) placeTotal *= size[x];

    // compute the global linear index
    int linearIndex = 0;
    for (int i = 0; i < int(index.size()); i++) {
        if (index[i] >= 0 && size[i] > 0 && index[i] < size[i]) {
            linearIndex = linearIndex * size[i];
            linearIndex += index[i];
        }
    }

    // compute #agents per place a.k.a. colonists
    int colonists = initPopulation / placeTotal;
    int remainders = initPopulation % placeTotal;
    if (linearIndex < remainders) colonists++;  // add a remainder

    return colonists;
};

/**
 * Initiates an agent migration upon a next call to Agents.manageAll( ). More
 * specifically, migrate( ) updates the calling agent’s index[].
 * @param index
 * @return
 */
bool Agent::migrate(vector<int> index) {
    vector<int> sizeVectors;
    sizeVectors = place->getSizeVect();
    for (int i = 0; (unsigned)i < sizeVectors.size(); i++) {
        if (index[i] >= 0 && index[i] < sizeVectors[i])
            continue;
        else
            return false;
    }
    this->index = index;
    return true;
}
