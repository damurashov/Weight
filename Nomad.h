#ifndef NOMAD_H
#define NOMAD_H

#include "Agent.h"
#include "Wave2D.h"

using namespace std;

class Nomad : public Agent {
public:
  static const int agentInit_ = 0;
  
  Nomad( void *argument ) : Agent( argument ) {
  };

  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case agentInit_: return agentInit( argument );
    }
    return NULL;
  };

private:
  void *agentInit( void *argument );
};

#endif
