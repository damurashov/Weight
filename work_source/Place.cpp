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
#include "MASS_base.h"

vector<int> Place::getSizeVect() { return size; }
/**
 * clears the neighbor vector
 */
void Place::cleanNeighbors() { neighbors.clear(); }
/**
 * replace neighbors completely using a vector of places
 */
void Place::addNeighbors(vector<Place *> places) {
    neighbors.clear();
    for (int i = 0; i < places.size(); i++) {
        neighbors.push_back(places.at(i));
    }
}
/**
 * Add a single neighbor to this place
 */
void Place::addNeighbor(Place *place) { neighbors.push_back(place); }
/**
 * adds neighbors in a given pattern if they exist.
 */
void Place::addNeighbors(neighborPattern pattern) {
    switch (pattern) {
        case FOUR: {
            // TODO implement indexing
            break;
        }
        case EIGHT: {
            // TODO implement indexing
            break;
        }
        default:
            break;
    }
}