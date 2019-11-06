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
#include <set>
#include <vector>
#include "MObject.h"

using namespace std;

class Place : MObject {
    friend class Places_base;
    friend class Agents_base;
    friend class Agent;

   public:
    /**
     * Is the default constructor. A contiguous space of arguments is passed
     * to the constructor.
     * @param argument
     */
    Place(void *argument)
        : outMessage(NULL), outMessage_size(0), inMessage_size(0) {
        inMessages.clear();
    };
    vector<int> getSizeVect();

    /**
     * Is called from Places.callAll( ), callSome( ), exchangeAll( ), and
     * exchangeSome( ), and invoke the function specified with functionId as
     * passing arguments to this function. A user-derived Place class must
     * implement this method.
     * @param functionId
     * @param argument
     * @return
     */
    virtual void *callMethod(int functionId, void *argument) = 0;

    // VARIABLES

    /**
     * Defines the size of the matrix that consists of application-specific
     * places. Intuitively, size[0], size[1], and size[2] correspond to the size
     * of x, y, and z, or that of i, j, and k.
     */
    vector<int> size;
    /**
     * Is an array that maintains each place’s coordinates. Intuitively,
     * index[0], index[1], and index[2] correspond to coordinates of x, y, and
     * z, or those of i, j, and k.
     */
    vector<int> index;

    /** Stores a set arguments to be passed to a set of remote-cell functions
     * that will be invoked by exchangeAll( ) or exchangeSome( ) in the
     * nearest future. The argument size must be specified with
     * outMessage_size.
     */
    void *outMessage;

    /** Receives a return value in inMessages[i] from a function call made to
     * the i-th remote cell through exchangeAll( ) and exchangeSome( ).
     * Each element size must be specified with inMessage_size.
     */
    vector<void *> inMessages;

    /**
     * Defines the size of outMessage.
     */
    int outMessage_size;

    /**
     * Defines the size of inMessage.
     */
    int inMessage_size;

    /** Includes all the agents residing locally on this place. */
    vector<MObject *> agents;

    // Methods to set, add, and remove neighbors.
    set<int *> getNeighbors() { return neighbors; };
    void addNeighbors(vector<int *> *destinations);
    void removeNeighbors(vector<int *> *destinations);
    bool isNeighbor(int *destination);

    ~Place(){};

   protected:
    void *getOutMessage(int handle, int index[]);
    void putInMessage(int handle, int index[], int position, void *value);

   private:
    Place *findDstPlace(int handle, int index[]);
    friend class Places_base;
    friend class Agents_base;
    friend class Agent;

   public:
    ~Place() { cleanNeighbors(); };

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
    vector<int *> getMooreNeighbors3d();

    vector<int *> getVNNeighbors2d();
    vector<int *> getVNNeighbors3d();
};

#endif