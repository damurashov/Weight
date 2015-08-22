#ifndef WAVE2DMASS_H
#define WAVE2DMASS_H

#include <string.h>
#include "Place.h"

using namespace std;

class Wave2DMass : public Place {
public:
  
  static const int init_ = 0;
  static const int computeWave_ = 1;
  static const int exchangeWave_ = 2;
  static const int collectWave_ = 3;
  double *wave;
  int time;
  double *neighbors;

  Wave2DMass( void *argument ) : Place( argument ) {
    wave = new double[3];
    wave[0]=wave[1]=wave[2]=0.0;
    time = 0;
    neighbors = new double[4];
  };
  
  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case init_: return init( argument );
    case computeWave_: return computeWave( argument );
    case exchangeWave_: return exchangeWave( argument );
    case collectWave_: return collectWave( argument );

    }
    return NULL;
  };

private:
  int sizeX, sizeY;
  int myX, myY;
  static const double c = 1.0;
  static const double dt = 0.1;
  static const double dd = 2.0;
  static const int north = 0, east = 1, south = 2, west = 3;
  void *init( void *argument );
  void *computeWave( void *argument );
  void *exchangeWave( void *argument );
  void *collectWave( void *argument );

};

#endif
