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

#include "Place.h"
#include <iostream>
#include <sstream>  // ostringstream
#include "MASS_base.h"
#include "limits.h"

/**
 * Returns the size of the matrix that consists of application-specific
 * places. Intuitively, size[0], size[1], and size[2] correspond to the size
 * of x, y, and z, or that of i, j, and k.
 *
 * @return  a collection of sizes that correspond to each dimension in the
 *          simulation space
 */
vector<int> Place::getSizeVect() { return size; }

/**
 * Method to retrieve the location (pointer) of a Place within the simulation.
 * The Place returned will exist relative to the Place represented by this
 * Object, using the 'handle' to ensure that they both live within the same
 * simulation space, and the value(s) passed as offset to determine the relative
 * location of the Place to retrieve.
 * 
 * @param handle  unique identifier for a set of Places in this simulation
 * @param offset  relative location of destination (Place) to retrieve output
 *                message from
 * @return        pointer to output message for this Place, else NULL
 */
Place *Place::findDstPlace(int handle, int offset[]) {
    // compute the global linear index from offset[]
    Places_base *places = MASS_base::placesMap[handle];
    int neighborCoord[places->dimension];
    places->getGlobalNeighborArrayIndex(index, offset, places->size,
                                        places->dimension, neighborCoord);
    int globalLinearIndex = places->getGlobalLinearIndexFromGlobalArrayIndex(
        neighborCoord, places->size, places->dimension);
    /*
    ostringstream convert;
    convert << "globalLinearIndex = " << globalLinearIndex << endl;
    MASS_base::log( convert.str( ) );
    */

    if (globalLinearIndex == INT_MIN) return NULL;

    // identify the destination place
    DllClass *dllclass = MASS_base::dllMap[handle];
    int destinationLocalLinearIndex =
        globalLinearIndex - places->lower_boundary;

    // return the destination outMessage
    return (dstPlace != NULL) ? dstPlace->outMessage : NULL;
}

/**
 *
 * @param handle
 * @param offset
 * @return
 */
void *Place::getOutMessage(int handle, int offset[]) {
    // fill out the space if inMessages are empty
    for (int i = 0; i <= position; i++)
        if (int(dstPlace->inMessages.size()) <= i &&
            dstPlace->inMessage_size > 0)
            dstPlace->inMessages.push_back(malloc(dstPlace->inMessage_size));

    // write to the destination inMessage[position]
    if (dstPlace != NULL && position < int(dstPlace->inMessages.size())) {
        memcpy(dstPlace->inMessages[position], value, dstPlace->inMessage_size);
    }
}
/**
 * return the vector containing this places neighbors
 */
vector<int *> Place::getNeighbors() { return this->neighbors; }
/**
 * clears the neighbor vector and fully clears the heap from any int[]'s stored
 * there
 */
void Place::cleanNeighbors() {
    for (int i = 0; i < neighbors.size(); i++) {
        if (neighbors.at(i)) {
            delete[](neighbors.at(i));
        }
    }
    neighbors.clear();
}
/**
 * replace neighbors completely using a vector of places
 */
void Place::addNeighbors(vector<int *> indexes) {
    cleanNeighbors();
    for (int i = 0; i < indexes.size(); i++) {
        neighbors.push_back(indexes.at(i));
    }
}
/**
 * Add a single neighbor to this place, neighbor vector should be created in the
 * heap, program will clean it up later, no need for the user to do so.
 */
void Place::addNeighbor(int *index) {
    // check for redundant connections?
    // for (int i = 0; i < neighbors.size(); i++) {
    //     if (neighbors.at(i)) {
    //     }
    neighbors.push_back(index);
}
/**
 * adds neighbors in a given pattern if they exist.
 */
void Place::addNeighbors(neighborPattern pattern) {
    cleanNeighbors();
    switch (pattern) {
        case MOORE2D: {
            neighbors = getMooreNeighbors2d();
            break;
        }
        case VON_NEUMANN2D: {
            neighbors = getVNNeighbors2d();
            break;
        }
        case MOORE3D: {
            neighbors = getMooreNeighbors3d();
            break;
        }
        case VON_NEUMANN3D: {
            neighbors = getVNNeighbors3d();
            break;
        }
        default:
            break;
    }
}

/**
 * Method to store an input message within a given Place of the simulation.
 * 
 * @param handle    unique identifier for a set of Places in this simulation
 * @param offset    relative location of destination (Place) to store input message
 * @param position  numerical index to place input message (in message array)
 * @param value     pointer to value to store in message array
 */
void Place::putInMessage( int handle, int offset[], int position,
    void *value ) {

    // fill out the space if inMessages are empty
    for (int i = 0; i <= position; i++)
        if (int(dstPlace->inMessages.size()) <= i &&
            dstPlace->inMessage_size > 0)
            dstPlace->inMessages.push_back(malloc(dstPlace->inMessage_size));

    // write to the destination inMessage[position]
    if (dstPlace != NULL && position < int(dstPlace->inMessages.size()))
        memcpy(dstPlace->inMessages[position], value, dstPlace->inMessage_size);
}

vector<int *> Place::getVNNeighbors2d() {
    vector<int *> result;
    // 4 neighbors for Mead
    int *x = new int[2]{1, 0};
    result.push_back(x);
    int *y = new int[2]{0, 1};
    result.push_back(y);
    int *_x = new int[2]{-1, 0};
    result.push_back(_x);
    int *_y = new int[2]{0, -1};
    result.push_back(_y);
    return result;
}
vector<int *> Place::getMooreNeighbors2d() {
    vector<int *> result;
    // 4 vectors covers all neighbors in 3d
    // 4 more neighbors for Moore
    result = getVNNeighbors2d();
    // 4 more for von neumann
    int *_xy = new int[2]{-1, 1};
    result.push_back(_xy);
    int *xy = new int[2]{1, 1};
    result.push_back(xy);
    int *_x_y = new int[2]{-1, -1};
    result.push_back(_x_y);
    int *x_y = new int[2]{1, -1};
    result.push_back(x_y);
    return result;
}
vector<int *> Place::getVNNeighbors3d() {
    vector<int *> result;
    // unit vectors only
    int *x = new int[3]{1, 0, 0};
    result.push_back(x);
    int *y = new int[3]{0, 1, 0};
    result.push_back(y);
    int *z = new int[3]{0, 0, 1};
    result.push_back(z);
    int *_x = new int[3]{-1, 0, 0};
    result.push_back(_x);
    int *_y = new int[3]{0, -1, 0};
    result.push_back(_y);
    int *_z = new int[3]{0, 0, -1};
    result.push_back(_z);
    return result;
}
vector<int *> Place::getMooreNeighbors3d() {
    vector<int *> result;
    // 26 neighbors for full coverage
    // unit vectors
    result = getMeadNeighbors3d();
    // x and y
    int *xy = new int[3]{1, 1, 0};
    result.push_back(xy);
    int *_x_y = new int[3]{-1, -1, 0};
    result.push_back(_x_y);
    int *_xy = new int[3]{-1, 1, 0};
    result.push_back(_xy);
    int *x_y = new int[3]{1, -1, 0};
    result.push_back(x_y);
    // y and z
    int *yz = new int[3]{0, 1, 1};
    result.push_back(yz);
    int *_y_z = new int[3]{0, -1, -1};
    result.push_back(_y_z);
    int *_yz = new int[3]{0, -1, 1};
    result.push_back(_yz);
    int *y_z = new int[3]{0, 1, -1};
    result.push_back(y_z);
    // x and z
    int *xz = new int[3]{1, 0, 1};
    result.push_back(xz);
    int *_x_z = new int[3]{-1, 0, -1};
    result.push_back(_x_z);
    int *_xz = new int[3]{-1, 0, 1};
    result.push_back(_xz);
    int *x_z = new int[3]{1, 0, -1};
    result.push_back(x_z);
    // all none zero
    int *xyz = new int[3]{1, 1, 1};
    result.push_back(xyz);
    int *_x_y_z = new int[3]{-1, -1, -1};
    result.push_back(_x_y_z);
    // single negative
    int *_xyz = new int[3]{-1, 1, 1};
    result.push_back(_xyz);
    int *x_yz = new int[3]{1, -1, 1};
    result.push_back(x_yz);
    int *xy_z = new int[3]{1, 1, -1};
    result.push_back(xy_z);
    // two negative
    int *_x_yz = new int[3]{-1, -1, 1};
    result.push_back(_x_yz);
    int *x_y_z = new int[3]{1, -1, -1};
    result.push_back(x_y_z);
    int *_xy_z = new int[3]{-1, 1, -1};
    result.push_back(_xy_z);
    return result;
}
