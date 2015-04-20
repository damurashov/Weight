#include "Place.h"
#include "MASS_base.h"
#include "limits.h"
#include <iostream>
#include <sstream> // ostringstream

/**
 * Returns the size of the matrix that consists of application-specific
 * places. Intuitively, size[0], size[1], and size[2] correspond to the size
 * of x, y, and z, or that of i, j, and k.
 * @return 
 */
vector<int> Place::getSizeVect(  ){
	return size;
}

/**
 * 
 * @param handle
 * @param offset
 * @return 
 */
Place *Place::findDstPlace( int handle, int offset[] ) {
  // compute the global linear index from offset[]
  Places_base *places = MASS_base::placesMap[ handle ];
  int neighborCoord[places->dimension];
  places->getGlobalNeighborArrayIndex( index, offset, places->size, 
				       places->dimension, neighborCoord );
  int globalLinearIndex 
    = places->getGlobalLinearIndexFromGlobalArrayIndex( neighborCoord,
							places->size,
							places->dimension );
  /*
  ostringstream convert;
  convert << "globalLinearIndex = " << globalLinearIndex << endl;
  MASS_base::log( convert.str( ) );
  */

  if ( globalLinearIndex == INT_MIN )
    return NULL;

  // identify the destination place
  DllClass *dllclass = MASS_base::dllMap[ handle ];
  int destinationLocalLinearIndex
    = globalLinearIndex - places->lower_boundary;

  Place *dstPlace = NULL;
  int shadow_index;
  if ( destinationLocalLinearIndex >= 0 && 
       destinationLocalLinearIndex < places->places_size )
    dstPlace = dllclass->places[ destinationLocalLinearIndex ];
  else if ( destinationLocalLinearIndex < 0 && 
       ( shadow_index = destinationLocalLinearIndex + places->shadow_size ) 
       >= 0 )
    dstPlace = dllclass->left_shadow[ shadow_index ];
  else if ( (shadow_index = destinationLocalLinearIndex - places->places_size)
	    >= 0
	    && shadow_index < places->shadow_size )
    dstPlace = dllclass->right_shadow[ shadow_index ];



  return dstPlace;
}

/**
 * Allows the user to add a list of neighbors to this specific place.
 * @param destinations - The neighbors to be added to the current neighbors list
 */
void Place::addNeighbors(vector<int*> *destinations) {
  int *tmp; // to hold elements from destinations

  // for each place in destinations, get the coordinate pair
  for ( int i = 0; i < int( destinations->size( ) ); i++ ) {
    tmp = (*destinations)[i];

    // Check that this location is not already present.
    if (!this->isNeighbor(tmp)) { // if this is not a neighbor

      // TODO: check this is a valid place
      // add to our vector of neighbors
      neighbors.push_back(tmp);
      // TODO: Figure out if we need to have the vector sorted to ensure 
      //       fast access.
      //       For example, if a Place had 1000 neighbors, and you needed the 
      //       997th one, do we search through the neighbors sequentially?

    }
  }
}

/**
 * Allows the user to remove a list of specified neighbors from this specific place.
 * Note that the neighbors can range
 * @param destinations - The neighbors to be removed from the current neighbors list
 */
void Place::removeNeighbors(vector<int*> *destinations) {
  int *tmp; // to hold elements in destinations

  // for each place in destinations, get the coordinate pair
  for ( int i = 0; i < int( destinations->size( ) ); i++ ) {
    tmp = (*destinations)[i];

    // check if that neighbor exists
    if (this->isNeighbor(tmp)) {
      neighbors.erase(neighbors.begin() + i);  // if it does, remove it.
    } else {
      // TODO: Add some form of error reporting here.

    }
  }
}

/**
 * Checks if a given neighbor exists in a Place's list of neighbors.
 * @param destination - The neighbor to check for
 * @return  true if the neighbor exists, false if it does not.
 */
bool Place::isNeighbor(int *destination) {
  int *tmp; // holds each neighbor in our neighbor's list

  // for each neighbor, check if a match is found.
  for (unsigned int i = 0; i < neighbors.size(); i++) {
    tmp = neighbors[i];  // get the next neighbor

    // if the destination is in the neighbor list, return true
    if (destination[0] == tmp[0] && destination[1] == tmp[1]) {
      return true;
    }
  }

  // if we reach this point, the neighbor was not found.
  return false;
}


/**
 * 
 * @param handle
 * @param offset
 * @return 
 */
void *Place::getOutMessage( int handle, int offset[] ) {

  Place *dstPlace = findDstPlace( handle, offset );

  // return the destination outMessage
  return ( dstPlace != NULL ) ? dstPlace->outMessage : NULL;

}

/**
 * 
 * @param handle
 * @param offset
 * @param position
 * @param value
 */
void Place::putInMessage( int handle, int offset[], int position, 
			  void *value ){

  Place *dstPlace = findDstPlace( handle, offset );

  // fill out the space if inMessages are empty
  for ( int i = 0; i <= position; i++ ) 
    if ( int( dstPlace->inMessages.size( ) ) 
	 <= i && dstPlace->inMessage_size > 0 )
      dstPlace->inMessages.push_back( malloc( dstPlace->inMessage_size ) );

  // write to the destination inMessage[position]
    if ( dstPlace != NULL && position < int( dstPlace->inMessages.size( ) ) ) 
    memcpy( dstPlace->inMessages[ position ], value, 
	    dstPlace->inMessage_size );
}
