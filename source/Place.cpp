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
 The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
 © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "Place.h"
#include "MASS_base.h"
#include "limits.h"
#include <iostream>
#include <sstream> // ostringstream

vector<int> Place::getSizeVect() {
	return size;
}

Place *Place::findDstPlace(int handle, int offset[]) {
	// compute the global linear index from offset[]
	Places_base *places = MASS_base::placesMap[handle];
	int neighborCoord[places->dimension];
	places->getGlobalNeighborArrayIndex(index, offset, places->size,
		places->dimension, neighborCoord);
	int globalLinearIndex
		= places->getGlobalLinearIndexFromGlobalArrayIndex(neighborCoord,
			places->size,
			places->dimension);
	/*
	ostringstream convert;
	convert << "globalLinearIndex = " << globalLinearIndex << endl;
	MASS_base::log( convert.str( ) );
	*/

	if (globalLinearIndex == INT_MIN)
		return NULL;

	// identify the destination place
	DllClass *dllclass = MASS_base::dllMap[handle];
	int destinationLocalLinearIndex
		= globalLinearIndex - places->lower_boundary;

	Place *dstPlace = NULL;
	int shadow_index;
	if (destinationLocalLinearIndex >= 0 &&
		destinationLocalLinearIndex < places->places_size)
		dstPlace = dllclass->places[destinationLocalLinearIndex];
	else if (destinationLocalLinearIndex < 0 &&
		(shadow_index = destinationLocalLinearIndex + places->shadow_size)
		>= 0)
		dstPlace = dllclass->left_shadow[shadow_index];
	else if ((shadow_index = destinationLocalLinearIndex - places->places_size)
		>= 0
		&& shadow_index < places->shadow_size)
		dstPlace = dllclass->right_shadow[shadow_index];



	return dstPlace;
}

void *Place::getOutMessage(int handle, int offset[]) {

	Place *dstPlace = findDstPlace(handle, offset);

	// return the destination outMessage
	return (dstPlace != NULL) ? dstPlace->outMessage : NULL;

}

void Place::putInMessage(int handle, int offset[], int position,
	void *value) {

	Place *dstPlace = findDstPlace(handle, offset);

	// fill out the space if inMessages are empty
	for (int i = 0; i <= position; i++)
		if (int(dstPlace->inMessages.size())
			<= i && dstPlace->inMessage_size > 0)
			dstPlace->inMessages.push_back(malloc(dstPlace->inMessage_size));

	// write to the destination inMessage[position]
	if (dstPlace != NULL && position < int(dstPlace->inMessages.size()))
		memcpy(dstPlace->inMessages[position], value,
			dstPlace->inMessage_size);
}
