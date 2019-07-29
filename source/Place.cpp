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

vector<int> Place::getSizeVect() { return size; }

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

    Place *dstPlace = NULL;
    int shadow_index;
    if (destinationLocalLinearIndex >= 0 &&
        destinationLocalLinearIndex < places->places_size)
        dstPlace = dllclass->places[destinationLocalLinearIndex];
    else if (destinationLocalLinearIndex < 0 &&
             (shadow_index =
                  destinationLocalLinearIndex + places->shadow_size) >= 0)
        dstPlace = dllclass->left_shadow[shadow_index];
    else if ((shadow_index =
                  destinationLocalLinearIndex - places->places_size) >= 0 &&
             shadow_index < places->shadow_size)
        dstPlace = dllclass->right_shadow[shadow_index];

    return dstPlace;
}

void *Place::getOutMessage(int handle, int offset[]) {
    Place *dstPlace = findDstPlace(handle, offset);

    // return the destination outMessage
    return (dstPlace != NULL) ? dstPlace->outMessage : NULL;
}

void Place::putInMessage(int handle, int offset[], int position, void *value) {
    Place *dstPlace = findDstPlace(handle, offset);

    // fill out the space if inMessages are empty
    for (int i = 0; i <= position; i++)
        if (int(dstPlace->inMessages.size()) <= i &&
            dstPlace->inMessage_size > 0)
            dstPlace->inMessages.push_back(malloc(dstPlace->inMessage_size));

    // write to the destination inMessage[position]
    if (dstPlace != NULL && position < int(dstPlace->inMessages.size()))
        memcpy(dstPlace->inMessages[position], value, dstPlace->inMessage_size);
}

/**
 * clears the neighbor vector and fully clears the heap from any int[]'s stored there
 */
void Place::cleanNeighbors() {
    for (int i = 0; i < neighbors.size(); i++) {
        if (neighbors.at(i)) {
            delete[] (neighbors.at(i));
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
 * Add a single neighbor to this place, neighbor vector should be created in the heap,
 * program will clean it up later, no need for the user to do so.
 */
void Place::addNeighbor(int *index) { 
    //check for redundant connections?
    for(int i = 0; i < neighbors.size(); i++){
        if(neighbors.at(i))
    }
    neighbors.push_back(index); }
/**
 * adds neighbors in a given pattern if they exist.
 */
void Place::addNeighbors(neighborPattern pattern) {
    cleanNeighbors();
    switch (pattern) {
        case MEAD2D: {
            neighbors = move(getMeadNeighbors2d());
            break;
        }
        case VON_NEUMANN2D: {
            neighbors = getVNNeighbors2d();
            break;
        }
        case MEAD3D: {
            neighbors = getMeadNeighbors3d();
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

vector<int *> Place::getMeadNeighbors2d() {
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
vector<int *> Place::getVNNeighbors2d() {
    vector<int *> result;
    // 8 vectors covers all neighbors in 3d
    // 4 neighbors for Mead
    result = getMeadNeighbors2d();
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
vector<int *> Place::getMeadNeighbors3d() {
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
vector<int *> Place::getVNNeighbors3d() {
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
