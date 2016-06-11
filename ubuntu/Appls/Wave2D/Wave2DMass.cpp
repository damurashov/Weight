#include "Wave2DMass.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

//Used to toggle output for Wave2d
const bool printOutput = false;
// const bool printOutput = true;

extern "C" Place* instantiate( void *argument ) {
  return new Wave2DMass( argument );
}

extern "C" void destroy( Place *object ) {
  delete object;
}

void *Wave2DMass::init( void *argument ) {

  inMessage_size = sizeof( double );
  //outMessage_size = sizeof( double );
  //outMessage = NULL;

  sizeX = size[0]; sizeY = size[1]; // size  is the base data members
  myX = index[0];  myY = index[1];  // index is the base data members
	
  // reset the neighboring area information.
  neighbors[north] = neighbors[east] = neighbors[south] = 
  neighbors[west] = 0.0;
  
  return NULL;
}

void *Wave2DMass::computeWave( void *argument ){
	time = *(int *)argument;
	
	// move the previous return values to my neighbors[].
	if ( !inMessages.empty() ) {
	  //cout<<inMessages.size()<<endl;
	  for ( size_t i = 0; i < inMessages.size(); i++ ){
		if (inMessages[i] != NULL){  
		  neighbors[i] = *(double *)inMessages[i];
		  delete (double *)inMessages[i];
		}
	  }
	  inMessages.clear();
	}

	if ( myX == 0 || myX == sizeX - 1 || myY == 0 || myY == sizeY ) {
	  // this cell is on the edge of the Wave2D matrix
	  if ( time == 0 )
		wave[0] = 0.0; //current
	  if ( time == 1 )
		wave[1] = 0.0; //previous
	  else if ( time >= 2 )
		wave[2] = 0.0; //previous2
	}
	else {
	  // this cell is not on the edge
	  if ( time == 0 ) {
		// create an initial high tide in the central square area
		wave[0] = 
		( sizeX * 0.4 <= myX && myX <= sizeX * 0.6 &&
		 sizeY * 0.4 <= myY && myY <= sizeY * 0.6 ) ? 20.0 : 0.0; //start w/ wave[0]
		wave[1] = wave[2] = 0.0; // init wave[1] and wave[2] as 0.0
	  }
	  else if ( time == 1 ) {
		// simulation at time 1 
		wave[1] = wave[0] +
		c * c / 2.0 * dt * dt / ( dd * dd ) *
		( neighbors[north] + neighbors[east] + neighbors[south] + 
		 neighbors[west] - 4.0 * wave[0] ); //wave[1] based on wave[0]

	  } 
	  else if ( time >= 2 ) { 
		// simulation at time 2 and onwards
		wave[2] = 2.0 * wave[1] - wave[0] +
		c * c * dt * dt / ( dd * dd ) *
		( neighbors[north] + neighbors[east] + neighbors[south] + 
		 neighbors[west] - 4.0 * wave[1] ); //wave two based on wave[1] and wave[0]
		wave[0] = wave[1]; wave[1] = wave[2]; //shift wave[] measurements, prepare for a new wave[2]
	  }
	}
	return NULL;

}

void *Wave2DMass::exchangeWave( void *argument ) {
  double *retVal = new double;
  //*retVal = *(double *)((time == 0) ? &wave[0] : &wave[1]);
  *retVal = (time == 0) ? wave[0] : wave[1];
  return retVal;
}

void *Wave2DMass::collectWave( void *argument ) {
  double *ret_val = new double;
  *ret_val = wave[2];
  
  return ret_val;
}

