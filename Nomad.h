#ifndef NOMAD_H
#define NOMAD_H

#include "Agent.h"
#include "Wave2D.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

using namespace std;

class Nomad : public Agent {

 public:
  static const int agentInit_ = 0;
  static const int somethingFun_ = 1;
  static const int createChild_ = 2;
  static const int killMe_ = 3;
  static const int move_ = 4;
  static const int callalltest_ = 5;
  
  Nomad( void *argument ) : Agent( argument ) {
    MASS_base::log( "BORN!!" );
  };

  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case agentInit_: return agentInit( argument );
    case somethingFun_: return somethingFun( argument );
    case createChild_: return createChild( argument );
    case killMe_: return killMe( argument );
    case move_: return move( argument );
    case callalltest_: return callalltest( argument );
    }
    return NULL;
  };

 private:
  void *agentInit( void *argument );
  void *somethingFun( void *argument );
  void *createChild( void *argument );
  void *killMe( void *argument );
  void *move( void *argument );
  void *callalltest( void *argument );

};

#endif
