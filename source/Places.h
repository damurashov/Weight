#ifndef PLACES_H
#define PLACES_H

#include <string>
#include <stdarg.h>  // int size, ...
#include "Places_base.h"
#include "Message.h"

using namespace std;

class Places: public Places_base {
public:
  Places( int handle, string className, void *argument, int argument_size,
      int dim, ... );
  Places( int handle, string className, void *argument, int argument_size,
      int dim, int size[] );

  Places( int handle, string className, int boundary_width, void *argument,
      int argument_size, int dim, ... );
  Places( int handle, string className, int boundary_width, void *argument,
      int argument_size, int dim, int size[] );

  ~Places( );

  void callAll( int functionId );
  void callAll( int functionId, void *argument, int arg_size );
  void *callAll( int functionId, void *argument[], int arg_size, int ret_size );
  void callSome( int functionId, int dim, int index[], int numPlaces );
  void callSome( int functionId, void *arguments[], int arg_size, int dim,
      int index[], int numPlaces );
  void *callSome( int functionId, void *arguments[], int arg_size, int ret_size,
      int dim, int index[], int numPlaces );
  void exchangeAll( int dest_handle, int functionId,
      vector<int*> *destinations );
  void exchangeBoundary( );
  void init_master( void *argument, int argument_size, int boundary_width );

private:
  void *ca_setup( int functionId, void *argument, int arg_size, int ret_size,
      Message::ACTION_TYPE type );
  void *cs_setup( int functionId, void *arguments, int arg_size, int ret_size,
      int dim, int index[], int numPlaces, Message::ACTION_TYPE type );
};

#endif
