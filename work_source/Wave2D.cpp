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

#include "Wave2D.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

//Used to toggle output for Wave2d
//const bool printOutput = false;
const bool printOutput = true;

extern "C" Place* instantiate( void *argument ) {
  return new Wave2D( argument );
}

extern "C" void destroy( Place *object ) {
  delete object;
}

void *Wave2D::init( void *argument ) {

  ostringstream convert;

  if(printOutput == true){
      convert << "init[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "]" << arg << (char *)argument;
      MASS_base::log( convert.str( ) );
  }

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
  
  if(printOutput == true){
      convert << "callalltest[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "]" << arg << ", " 
	      << *(int  *)argument
	      << " *(double *)ret_val = " << *ret_val;
      MASS_base::log( convert.str( ) );
  }
  return ret_val;
}

void *Wave2D::exchangetest( void *argument ) {
  ostringstream convert;

  if(printOutput == true){
      convert << "exchangetest[" << index[0] << "][" << index[1] << "] out of ["
	      << size[0] << "][" << size[1] << "] received " << *(int *)argument;
      MASS_base::log( convert.str( ) );
  }

  int *retVal = new int;
  *retVal = *(int *)argument * 10000 + index[0] * 100 + index[1];
  
  return retVal;
}
void *Wave2D::checkInMessage( void *argument ) {

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
