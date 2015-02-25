#ifndef PLACES_BASE_H
#define PLACES_BASE_H

#include <string>
#include <vector>
#include "Place.h"

using namespace std;

class Places_base {
  friend class MProcess;
  friend class Agents_base;
  friend class Place;
public:
  Places_base( int handle, string className, int boundary_width, void *argument,
      int argument_size, int dim, int size[] );
  ~Places_base( );

  void callAll( int functionId, void *argument, int tid );
  void **callAll( int functionId, void *argument, int arg_size, int ret_size,
      int tid );
  void exchangeAll( Places_base *dstPlaces, int functionId,
      vector<int*> *destinations, int tid );
  void exchangeBoundary( );      // called from Places.exchangeBoundary( ) 

  int getHandle( ) {
    return handle;
  };
  int getPlacesSize( ) {
    return places_size;
  };

protected:
  /**
   * A unique identifer that designates a group of places. Must be unique over
   * all machines used in simulation space (hosts).
   */
  const int handle;

  /**
   * Name of the user-created Place classes to load.
   */
  const string className;

  /**
   * The number of dimensions this simulation space encompasses
   */
  const int dimension;

  int lower_boundary;
  int upper_boundary;
  int places_size;
  /**
   * Defines the size of each dimension in the simulation space. Intuitively,
   * size[0], size[1], and size[2] correspond to the size of x, y, and z, or
   * that of i, j, and k.
   */
  int *size;
  int shadow_size;
  int boundary_width;

  void init_all( void *argument, int argument_size );
  vector<int> getGlobalArrayIndex( int singleIndex );
  void getLocalRange( int range[], int tid );
  static void *processRemoteExchangeRequest( void *param );
  void getGlobalNeighborArrayIndex( vector<int> src_index, int offset[],
      int dst_size[], int dst_dimension, int *dest_index );
  int getGlobalLinearIndexFromGlobalArrayIndex( int dest_index[],
      int dest_size[], int dest_dimension );
  int getRankFromGlobalLinearIndex( int globalLinearIndex );
  static void *exchangeBoundary_helper( void *param );
  static void *sendMessageByChild( void *param );
  struct ExchangeSendMessage {
    int rank;
    void *message;
  };
};

#endif
