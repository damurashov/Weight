#include "MASS.h"
#include "Land.h"
#include "Nomad.h"
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>

//Used to toggle output for Main
const bool printOutput = false;
//const bool printOutput = true;

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
  Places *land = new Places( 1, "Land", msg, 7, 2, 100, 100 );

  msg = "good\0";
  land->callAll( Land::init_, msg, 5 );

  int callargs[100][100];
  for ( int i = 0; i < 100; i++ )
    for ( int j = 0; j < 100; j++ )
      callargs[i][j] = i * 100 + j;
  double *retvals = 
    (double *)
    land->callAll( Land::callalltest_, (void **)callargs, sizeof( int ),
		   sizeof( double ) );

  if(printOutput == true){
      for ( int i = 0; i < 100; i++ )
        for ( int j = 0; j < 100; j++ )
          cout << retvals[i * 100 + j] << endl;
  }
  delete retvals;

  vector<int*> destinations;
  int north[2] = {0, 1};  destinations.push_back( north );
  int east[2]  = {1, 0};  destinations.push_back( east );
  int south[2] = {0, -1}; destinations.push_back( south );
  int west[2]  = {-1, 0}; destinations.push_back( west );
    
  land->exchangeAll( 1, Land::exchangetest_, &destinations );

  land->callAll( Land::checkInMessage_ );

  Agents *nomad = new Agents( 2, "Nomad", msg, 7, land, 10000 );
  nomad->callAll( Nomad::agentInit_, msg, 5 );

  //Test callAll second time
  msg = "Second attempt\0";
  nomad->callAll( Nomad::somethingFun_, msg, 15 );

  //Test callAll with return values
  int agent_callargs[10000];
  for ( int i = 0; i < 10000; i++ )
    agent_callargs[i] = i;
  retvals = (double *)
    nomad->callAll( Nomad::callalltest_, (void *)agent_callargs,
		    sizeof( int ), sizeof( double ) );

  if(printOutput == true){
      for ( int i = 0; i < 10000; i++ )
        cout << retvals[i] << endl;
  }
  delete retvals;

  //Test manageAll
  nomad->callAll( Nomad::createChild_ );
  nomad->callAll( Nomad::killMe_ );
  nomad->callAll( Nomad::move_ );
  nomad->manageAll( );
  if ( printOutput == true )
    cout << "first manageAll done" << endl;

  //Test callAll with return values
  int agent_callargs2[15000];
  for ( int i = 0; i < 15000; i++ )
    agent_callargs2[i] = -i;

  if ( printOutput == true )
    cout << "nomad->callAll( callalltest ) starts" << endl;

  retvals = (double *)
    nomad->callAll( Nomad::callalltest_, (void *)agent_callargs2,
		    sizeof( int ), sizeof( double ) );

  if(printOutput == true){
      for ( int i = 0; i < 15000; i++ )
        cout << retvals[i] << endl;
  }
  delete retvals;

  nomad->callAll( Nomad::addData_ );
  nomad->callAll( Nomad::move2_ );
  nomad->manageAll( );

  MASS::finish( );
}