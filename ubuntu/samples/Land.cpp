#include "Land.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

//Used to toggle output for Wave2d
//const bool printOutput = false;
 const bool printOutput = true;

extern "C" Place* instantiate( void *argument ) {
  return new Land( argument );
}

extern "C" void destroy( Place *object ) {
  delete object;
}


/**
 * Initializes a Land object.
 */
void *Land::init( void *argument ) {

  ostringstream convert;

  if(printOutput == true){
      convert << "init[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "]" << arg << (char *)argument;
      MASS_base::log( convert.str( ) );
  }

  inMessage_size = sizeof( int );  // defines the size of the inMessagge.
  outMessage_size = sizeof( int ); // defines the size of the outMessage.
  outMessage = new int( );
  *(int *)outMessage = index[0] * 100 + index[1];
  
  return NULL;
}

/**
 * callalltest causes all Land's to report their location and contents.
 */
void *Land::callalltest( void *argument ) {
  ostringstream convert;
  
  double *ret_val = new double;
  *ret_val = *(int *)argument * 10.0;
  
  if(printOutput == true){
      convert << "callalltest[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "]" << arg << ", " 
	      << *(int  *)argument
	      << " *(double *)ret_val = " << *ret_val;
      MASS_base::log( convert.str( ) );
  }
  return ret_val;
}

/**
 * exchangetest causes each place to exchange information with it's neighbors.
 */
void *Land::exchangetest( void *argument ) {
  ostringstream convert;

  if(printOutput == true){
      convert << "exchangetest[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "] received "; //<< *(int *)argument;
      MASS_base::log( convert.str( ) );
  }

  int *retVal = new int;
  *retVal = *(int *)argument * 10000 + index[0] * 100 + index[1];
  
  return retVal;
}

/**
 * Logs any inMessages associated with the Place.
 */
void *Land::checkInMessage( void *argument ) {

  ostringstream convert;

  if(printOutput == true){
      convert << "checkInMessage[" << index[0] << "][" << index[1] 
	      << "] out of ["
	      << size[0] << "][" << size[1]
	      << "] inMessages.size = " << inMessages.size( )
	      << " received ";
      for ( int i = 0; i < int( inMessages.size( ) ); i++ )
        convert << " [" << i << "] = " << *(int *)(inMessages[i]);
  
      MASS_base::log( convert.str( ) );
  }

  return NULL;
}

/**
 */
void *Land::printOutMessage( void *argument ) {
  ostringstream convert;
  convert << "printOutMessage Land[" << index[0] << "][" << index[1] 
	  << "]'s outMessage = " << *(int *)outMessage;
  MASS_base::log( convert.str( ) );

  return NULL;
}

/**
 * Prints out the neighbors defined below if they exist.
 */
void *Land::printShadow( void *argument ) {
  int shadow[4];
  int north[2] = {0, 1}; 
  int east[2] = {1, 0}; 
  int south[2] = {0, -1};
  int west[2] = {-1, 0};
  int *ptr = (int *)getOutMessage( 1, north );
  shadow[0] = ( ptr == NULL ) ? 0 : *ptr;
  ptr = (int *)getOutMessage( 1, east );
  shadow[1] = ( ptr == NULL ) ? 0 : *ptr;
  ptr = (int *)getOutMessage( 1, south );
  shadow[2] = ( ptr == NULL ) ? 0 : *ptr;
  ptr = (int *)getOutMessage( 1, west );
  shadow[3] = ( ptr == NULL ) ? 0 : *ptr;

  ostringstream convert;
  convert << "printShadow:  Land[" << index[0] << "][" << index[1] 
	  << "]'s north = " << shadow[0] << ", east = " << shadow[1]
	  << ", south = " << shadow[2] << ", west = " << shadow[3];
  MASS_base::log( convert.str( ) );

  return NULL;
}


/**
 * Adds destinations (neighbors) to given Places.
 */
void *Land::addDestinations( void *argument ) {

  // TODO: Figure out how to grab from argument
  

  return NULL;
}
