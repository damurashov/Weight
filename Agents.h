#ifndef AGENTS_H
#define AGENTS_H

#include <string>
#include "Agents_base.h"
#include "Places.h"
#include "Message.h"

using namespace std;

class Agents : public Agents_base {
 public:
  Agents( int handle, string className, void *argument, int argument_size,
	  Places *places, int initPopulation );
  ~Agents( );

  void callAll( int functionId );
  void callAll( int functionId, void *argument, int arg_size );
  void *callAll( int functionId, void *argument[], int arg_size, 
		 int ret_size );
  void manageAll( );
  int nAgents( );
  
  void init_master( void *argument, int argument_size );
  
 private:
  int *localAgents;
  int total;
  void *ca_setup( int functionId, void *argument, int arg_size, int ret_size,
		  Message::ACTION_TYPE type );
};

#endif
