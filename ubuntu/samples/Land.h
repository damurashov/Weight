#ifndef Land_H
#define Land_H

#include <string.h>
#include "Place.h"

using namespace std;

class Land : public Place {
public:
  static const int init_ = 0;
  static const int callalltest_ = 1;
  static const int exchangetest_ = 2;
  static const int checkInMessage_ = 3;
  static const int printOutMessage_ = 4;
  static const int printShadow_ = 5;

  Land( void *argument ) : Place( argument ) {
    bzero( arg, sizeof( arg ) );
    strcpy( arg, (char *)argument );
  };
  
  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case init_: return init( argument );
    case callalltest_: return callalltest( argument );
    case exchangetest_: return exchangetest( argument );
    case checkInMessage_: return checkInMessage( argument );
    case printOutMessage_: return printOutMessage( argument );
    case printShadow_: return printShadow( argument );
    }
    return NULL;
  };

private:
  char arg[100];
  void *init( void *argument );
  void *callalltest( void *argument );
  void *exchangetest( void *argument );
  void *checkInMessage( void *argument );
  void *printOutMessage( void *argument );
  void *printShadow( void *argument );
};

#endif
