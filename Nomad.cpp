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


