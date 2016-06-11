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

#include "Nomad.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

//Toggles output for user program
// const bool printOutput = false;
const bool printOutput = true;

extern "C" Agent* instantiate( void *argument ) {
  return new Nomad( argument );
}

extern "C" void destroy( Agent *object ) {
  delete object;
}

void *Nomad::agentInit( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
      convert << "agentInit[" << agentId << "] called, argument = " << (char *)argument;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::somethingFun( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
      convert << "somethingFun[" << agentId << "] called, argument = " << (char *)argument;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::createChild( void *argument ) {
  int nChildren = 0;
  vector<void *> *arguments = new vector<void *>;
  arguments->push_back( argument );
  arguments->push_back( argument );
  if ( agentId % 2 == 0 ) {
    nChildren = 2;
    spawn( nChildren, *arguments, 15 );
  }
  delete arguments;

  ostringstream convert;
  if(printOutput == true){
      convert << "createChild[" << agentId << "] called to spawn " << nChildren;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::killMe( void *argument ) {
  if ( agentId % 2 != 0 ) {
    kill( );
  }

  ostringstream convert;
  if(printOutput == true){
      convert << "killMe[" << agentId << "] called";
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::move( void *argument ) {
  if ( agentId % 5 == 0 ) {
    vector<int> dest;
    dest.push_back( 0 );
    dest.push_back( 0 );
    migrate( dest );
  }

  ostringstream convert;
  if(printOutput == true){
      convert << "migrate[" << agentId << "] called";
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::callalltest( void *argument ) {
  ostringstream convert;
  if(printOutput == true){
      convert << "callalltest: agent(" << agentId;
      MASS_base::log( convert.str( ) );    
  }
  double *ret_val = new double;
  *ret_val = *(int *)argument * 10.0;

  if(printOutput == true){
      convert.str( "" );
      convert << "callalltest: agent(" << agentId << "): argument = "
              << *(int  *)argument
              << " *(double *)ret_val = " << *ret_val;
      MASS_base::log( convert.str( ) );
  }
  return ret_val;
}

void *Nomad::addData( void *argument ) {
  migratableDataSize = 24;
  migratableData = (void *)(new char[migratableDataSize]);
  bzero( migratableData, migratableDataSize );
  
  ostringstream convert;
  if(printOutput == true){
      convert << "my agent id = " << agentId;
  }

  memcpy( migratableData, (void *)( convert.str( ).c_str( ) ), 
          migratableDataSize );

  if(printOutput == true){
      convert << " dataSize = " << migratableDataSize;
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}

void *Nomad::move2( void *argument ) {

  int x = ( place->index[0] + 20 ) % 100;
  vector<int> dest;
  dest.push_back( x );
  dest.push_back( 0 );
  migrate( dest );  

  ostringstream convert;
  if(printOutput == true){
      convert << "my agent(" << agentId << ") will move from "
	      << "[" << place->index[0] << "][" << place->index[1]
	      << "] to [" << x << "][0]";
      MASS_base::log( convert.str( ) );
  }
  return NULL;
}
