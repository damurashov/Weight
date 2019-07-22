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

#ifndef PLACE_H
#define PLACE_H

#include <iostream>
#include <vector>
#include "MObject.h"

using namespace std;

class Place : MObject {
    friend class Places_base;
    friend class Agents_base;
    friend class Agent;

   public:
    Place(void *argument)
        : outMessage(NULL), outMessage_size(0), inMessage_size(0) {
        inMessages.clear();
    };
    // rename patterns according to specifications
    enum neighborPattern { FOUR, EIGHT };

    vector<int> getSizeVect();
    virtual void *callMethod(int functionId, void *argument) = 0;
    ~Place(){};

    vector<int> size;
    vector<int> index;
    void *outMessage;
    vector<void *> inMessages;
    int outMessage_size;
    int inMessage_size;
    vector<MObject *> agents;
    vector<Place *> neighbors;

    void cleanNeighbors();
    void addNeighbor(Place *place);
    void addNeighbors(vector<Place *>);
    void addNeighbors(neighborPattern pattern);

    // add vector for neighbors, implement in places_base, exchange all recives
    // neighboring information, make new exchange all that uses neighbors vector
    // instead of passed information

    // cleanNeighbors()
    // addNeighbors(vector)
    // addNeighbors(Enumeration of different neighbor patterns)
    // exchange all implementation in places basea
};

#endif
