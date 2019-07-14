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
presentations, etc.: © 2014-2015 University of Washington. MASS was developed by
Computing and Software Systems at University of Washington Bothell. THE SOFTWARE
IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Agent.h"
#include <iostream>
#include <sstream>  // ostringstream
#include "MASS_base.h"

// Set number for spawning additional Agents

Agent::~Agent() {
    if (migratableData != NULL) free(migratableData);
};
int map(int initPopulation, vector<int> size, vector<int> index) {
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

void Agent::spawn(int numAgents, vector<void*> arguments, int arg_size) {
    // Only want to make changes if the number to be created is above zero
    if (numAgents > 0) {
        newChildren = numAgents;
        this->arguments = arguments;
    }
}

// Set index for an Agent to migrate to
bool Agent::migrate(vector<int> index) {
    vector<int> sizeVectors;
    sizeVectors = place->getSizeVect();
    for (int i = 0; (unsigned)i < sizeVectors.size() - 1; i++) {
        if (index[i] >= 0 && index[i] < sizeVectors[i])
            continue;
        else
            return false;
    }
    this->index = index;
    return true;
}

int Agent::getAgentId() { return this->agentId; }
vector<void*> Agent::getArguments() { return this->arguments; }
vector<int> Agent::getIndex() { return this->index; }
int Agent::getNewChildren() { return this->newChildren; }
Place* Agent::getPlace() { return this->place; }
bool Agent::isAlive() { return isAlive; }
