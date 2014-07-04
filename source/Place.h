#ifndef PLACE_H
#define PLACE_H

#include "MObject.h"
#include <vector>
#include <iostream>

using namespace std;

class Place : MObject {
  friend class Places_base;
  friend class Agents_base;
  friend class Agent;
 public:
  Place( void *argument ) : outMessage( NULL ), outMessage_size( 0 ), 
    inMessage_size( 0 ) { inMessages.clear( ); };
 
  vector<int> getSizeVect( );
  virtual void *callMethod( int functionId, void *argument ) = 0;
  ~Place( ) { };

  vector<int> size;
  vector<int> index;
  void *outMessage;
  vector<void*> inMessages;
  int outMessage_size;
  int inMessage_size;
  vector<MObject*> agents;

 protected:
  void *getOutMessage( int handle, int index[] );
  void putInMessage( int handle, int index[], int position, void *value );

 private:
  Place *findDstPlace( int handle, int index[] );
};

#endif
