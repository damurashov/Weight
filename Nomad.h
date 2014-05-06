#ifndef NOMAD_H
#define NOMAD_H

#include "Agent.h"
#include "Wave2D.h"

using namespace std;

class Nomad : public Agent {
public:
  static const int agentInit_ = 0;
  static const int somethingFun_ = 1;
  static const int createChild_ = 2;
  static const int killMe_ = 3;
  
  Nomad( void *argument ) : Agent( argument ) {
  };

  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case agentInit_: return agentInit( argument );
    case somethingFun_: return somethingFun( argument );
    case createChild_: return createChild( argument );
    case killMe_: return killMe( argument );
    }
    return NULL;
  };

private:
  void *agentInit( void *argument );
  void *somethingFun( void *argument );
  void *createChild( void *argument );
  void *killMe( void *argument );
};

#endif
