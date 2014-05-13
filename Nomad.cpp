#include "Nomad.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream

extern "C" Agent* instantiate( void *argument ) {
  return new Nomad( argument );
}

extern "C" void destroy( Agent *object ) {
  delete object;
}

void *Nomad::agentInit( void *argument ) {
  ostringstream convert;
  convert << "agentInit[" << agentId << "] called, argument = " << (char *)argument;
  MASS_base::log( convert.str( ) );

  return NULL;
}

void *Nomad::somethingFun( void *argument ) {
  ostringstream convert;
  convert << "somethingFun[" << agentId << "] called, argument = " << (char *)argument;
  MASS_base::log( convert.str( ) );

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
  convert << "createChild[" << agentId << "] called to spawn " << nChildren;
  MASS_base::log( convert.str( ) );

  return NULL;
}

void *Nomad::killMe( void *argument ) {
  if ( agentId % 2 != 0 ) {
    kill( );
  }

  ostringstream convert;
  convert << "killMe[" << agentId << "] called";
  MASS_base::log( convert.str( ) );

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
  convert << "migrate[" << agentId << "] called";
  MASS_base::log( convert.str( ) );

  return NULL;
}

void *Nomad::callalltest( void *argument ) {
  ostringstream convert;
  convert << "callalltest: agent(" << agentId;
  MASS_base::log( convert.str( ) );    

  double *ret_val = new double;
  *ret_val = *(int *)argument * 10.0;

  convert.str( "" );
  convert << "callalltest: agent(" << agentId << "): argument = "
          << *(int  *)argument
          << " *(double *)ret_val = " << *ret_val;
  MASS_base::log( convert.str( ) );

  return ret_val;
}

void *Nomad::addData( void *argument ) {
  migratableDataSize = 24;
  migratableData = (void *)(new char[migratableDataSize]);
  bzero( migratableData, migratableDataSize );
  
  ostringstream convert;
  convert << "my agent id = " << agentId;
  memcpy( migratableData, (void *)( convert.str( ).c_str( ) ), 
	  migratableDataSize );

  convert << " dataSize = " << migratableDataSize;
  MASS_base::log( convert.str( ) );

  return NULL;
}

void *Nomad::move2( void *argument ) {

  int x = ( place->index[0] + 20 ) % 100;
  vector<int> dest;
  dest.push_back( x );
  dest.push_back( 0 );
  migrate( dest );  

  ostringstream convert;
  convert << "my agent(" << agentId << ") will move from "
	  << "[" << place->index[0] << "][" << place->index[1]
	  << "] to [" << x << "][0]";
  MASS_base::log( convert.str( ) );

  return NULL;
}
