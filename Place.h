#ifndef PLACE_H
#define PLACE_H

#include "MObject.h"
#include <vector>
#include <iostream>

using namespace std;

class Place : MObject {
  friend class Places_base;
  friend class Agents_base;
 public:
  Place( void *argument ) : outMessage( NULL ), outMessage_size( 0 ), 
    inMessage_size( 0 ) { inMessages.clear( ); };
 
  vector<int> getSizeVect( );
    // virtual void *callMethod( int functionId, void *argument ) = 0;

    // virtual ~Place( ) { };
//  vector<int> size;
 protected:
  vector<int> size;
  vector<int> index;
  void *outMessage;
  vector<void*> inMessages;
  int outMessage_size;
  int inMessage_size;
  vector<MObject*> agents;
};

#endif
