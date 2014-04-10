#include "Wave2D.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

extern "C" Place* instantiate( void *argument ) {
  return new Wave2D( argument );
}

extern "C" void destroy( Place *object ) {
  delete object;
}

void *Wave2D::init( void *argument ) {

  ostringstream convert;
  convert << "init[" << index[0] << "][" << index[1] << "] out of ["
	  << size[0] << "][" << size[1] << "]" << arg << (char *)argument;
  MASS_base::log( convert.str( ) );
  
  inMessage_size = sizeof( int );
  outMessage_size = sizeof( int );
  outMessage = new int( );
  *(int *)outMessage = index[0] * 100 + index[1];
  
  return NULL;
}

void *Wave2D::callalltest( void *argument ) {
  ostringstream convert;
  
  double *ret_val = new double;
  *ret_val = *(int *)argument * 10.0;
  
  convert << "callalltest[" << index[0] << "][" << index[1] << "] out of ["
	  << size[0] << "][" << size[1] << "]" << arg << ", " 
	  << *(int  *)argument
	  << " *(double *)ret_val = " << *ret_val;
  MASS_base::log( convert.str( ) );
  
  return ret_val;
}

void *Wave2D::exchangetest( void *argument ) {
  ostringstream convert;
  convert << "exchangetest[" << index[0] << "][" << index[1] << "] out of ["
	  << size[0] << "][" << size[1] << "] received " << *(int *)argument;
  MASS_base::log( convert.str( ) );
  
  int *retVal = new int;
  *retVal = *(int *)argument * 10000 + index[0] * 100 + index[1];
  
  return retVal;
}
void *Wave2D::checkInMessage( void *argument ) {

  ostringstream convert;
  convert << "checkInMessage[" << index[0] << "][" << index[1] 
	  << "] out of ["
	  << size[0] << "][" << size[1]
	  << "] inMessages.size = " << inMessages.size( )
	  << " received ";
  for ( int i = 0; i < int( inMessages.size( ) ); i++ )
    convert << " [" << i << "] = " << *(int *)(inMessages[i]);
  
  MASS_base::log( convert.str( ) );
  
  return NULL;
}

