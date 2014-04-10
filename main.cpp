#include "MASS.h"
#include "Wave2D.h"
#include "Nomad.h"
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>

int main( int argc, char *args[] ) {

  if ( argc != 7 ) {
    cerr << "usage: ./main username password machinefile port nProc nThr" 
	 << endl;
    return -1;
  }
  char *arguments[4];
  arguments[0] = args[1]; // username
  arguments[1] = args[2]; // password
  arguments[2] = args[3]; // machinefile
  arguments[3] = args[4]; // port
  int nProc = atoi( args[5] );
  int nThr = atoi( args[6] );
    
  MASS::init( arguments, nProc, nThr );
  char *msg = "hello\0"; // should not be char msg[]
  Places *wave2d = new Places( 1, "Wave2D", msg, 7, 2, 10, 10 );

  msg = "good\0";
  wave2d->callAll( Wave2D::init_, msg, 5 );

  int callargs[100][100];
  for ( int i = 0; i < 100; i++ )
    for ( int j = 0; j < 100; j++ )
      callargs[i][j] = i * 100 + j;
  double *retvals = 
    (double *)
    wave2d->callAll( Wave2D::callalltest_, (void **)callargs, sizeof( int ),
		   sizeof( double ) );

  for ( int i = 0; i < 100; i++ )
    for ( int j = 0; j < 100; j++ )
      cout << retvals[i * 100 + j] << endl;

  vector<int*> destinations;
  int north[2] = {0, 1};  destinations.push_back( north );
  int east[2]  = {1, 0};  destinations.push_back( east );
  int south[2] = {0, -1}; destinations.push_back( south );
  int west[2]  = {-1, 0}; destinations.push_back( west );
    
  wave2d->exchangeAll( 1, Wave2D::exchangetest_, &destinations );

  wave2d->callAll( Wave2D::checkInMessage_ );

  Agents *nomad = new Agents( 2, "Nomad", msg, 7, wave2d, 5000 );
  nomad->callAll( Nomad::agentInit_, msg, 5 );

  MASS::finish( );
}
