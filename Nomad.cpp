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


