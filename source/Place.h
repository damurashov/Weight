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
    Place(void *argument) : inMessage_size(0) {
        outMessage = new int;
        *(int *)outMessage = 0;
        outMessage_size = sizeof(int);
        inMessages.clear();
    };

    vector<int> getSizeVect();
    virtual void *callMethod(int functionId, void *argument) = 0;
    ~Place() { cleanNeighbors(); };

    vector<int> size;
    vector<int> index;
    void *outMessage;
    vector<void *> inMessages;
    int outMessage_size;
    int inMessage_size;
    vector<MObject *> agents;

    enum neighborPattern {
        VON_NEUMANN2D,
        MOORE2D,
        VON_NEUMANN3D,
        MOORE3D,
    };

    vector<int *> getNeighbors();

    void cleanNeighbors();
    void addNeighbor(int *);
    void addNeighbors(vector<int *>);
    void addNeighbors(neighborPattern pattern);

   private:
    vector<int *> neighbors;
    vector<int *> getMooreNeighbors2d();
    vector<int *> getVNNeighbors2d();
    vector<int *> getMooreNeighbors3d();
    vector<int *> getVNNeighbors3d();

    Place *findDstPlace(int handle, int index[]);

   protected:
    void *getOutMessage(int handle, int index[]);
    void putInMessage(int handle, int index[], int position, void *value);
};

#endif