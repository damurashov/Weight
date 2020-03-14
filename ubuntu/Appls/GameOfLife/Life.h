
#ifndef Life_H
#define Life_H

#include <string.h>
#include "Place.h"

using namespace std;

class Life : public Place {
public:

	static const int NUM_NEIGHBORS = 8;									//A life has 8 neighbors
  
  // define functionId's that will 'point' to the functions they represent.
  static const int init_ = 0;
  static const int computeDeadOrAlive_ = 1;
  static const int displayHealthStatus_ = 2;
  static const int getBoundaryHealthStatus_ = 3;

  
  /**
   * Initialize a Life object by allocating memory for it.
   */
  Life( void *argument ) : Place( argument ) {
    bzero( arg, sizeof( arg ) );
    strcpy( arg, (char *)argument );
  };
  
  /**
   * the callMethod uses the function ID to determine which method to execute.
   * It is assumed the arguments passed in contain everything those 
   * methods need to run.
   */
  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
      case init_: return init( argument );
      case computeDeadOrAlive_: return computeDeadOrAlive();
      case displayHealthStatus_: return displayHealthStatus();
      case getBoundaryHealthStatus_: return getBoundaryHealthStatus();
    }
    return NULL;
  };

private:
  char arg[100];
  void *init( void *argument );
  void *computeDeadOrAlive();
  void *displayHealthStatus();										//Set the outmessage as population of this life
  
  //PartitionMove
  void *getBoundaryHealthStatus();									//Based on neighbor population, calculate nextMove and save it 		#partition-space
  
  vector<int*> cardinals;										//Vector form of cardinals
  static const int neighbor[8][2];								//Array form of cardinals
  bool alive;                                      //Status of whether current place is ALIVE OR DEAD
  int neighborHealthStatus[8];											//An array to store int health status of neighbors
};

#endif
