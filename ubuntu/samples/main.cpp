#include "MASS.h"
#include "Land.h"
#include "Nomad.h"
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>

//Used to toggle output for Main
//const bool printOutput = false;
const bool printOutput = true;

int main( int argc, char *args[] ) {

  // check that all arguments are present 
  // exit with an error if they are not
  if ( argc != 7 ) {
    cerr << "usage: ./main username password machinefile port nProc nThr" 
	 << endl;
    return -1;
  }
  
  // get the arguments passed in
  char *arguments[4];
  arguments[0] = args[1]; // username
  arguments[1] = args[2]; // password
  arguments[2] = args[3]; // machinefile
  arguments[3] = args[4]; // port
  int nProc = atoi( args[5] ); // number of processes
  int nThr = atoi( args[6] );  // number of threads
    
  
  // initialize MASS with the machine information,
  // number of processes, and number of threads
  MASS::init( arguments, nProc, nThr );
  
  // prepare a message for the places (this is argument below)
  char *msg = (char *)("hello\0"); // should not be char msg[]
  
  /*  THIS SECTION OF CODE DEALS ONLY WITH PLACES  */
  
  // Create the places.
  // Arguments are, in order:
  //    handle, className, boundary_width, argument, argument_size, dim, ...
  Places *land = new Places( 1, "Land", 1, msg, 7, 2, 100, 100 );

  // change message to good
  msg = (char *)("good\0");
  
  // call the Land class's implementation of callAll 
  // with the msg and the length of the message.
  land->callAll( Land::init_, msg, 5 );

  // Create and populate and array of ints to represent locations.
  int callargs[100][100];
  for ( int i = 0; i < 100; i++ )
    for ( int j = 0; j < 100; j++ )
      callargs[i][j] = i * 100 + j;
  
  // create a double to hold the return values from the callalltest_.
  double *retvals = 
    (double *)
    land->callAll( Land::callalltest_, (void **)callargs, sizeof( int ),
		   sizeof( double ) );

  // print the contents of the locations.
  if(printOutput == true){
      for ( int i = 0; i < 100; i++ )
        for ( int j = 0; j < 100; j++ )
          cout << retvals[i * 100 + j] << endl;
  }
  delete retvals;
  
  // define the destinations, which represent the Places 
  // adjacent to a particular place (represented by [0, 0].
  //        [0, 1]                 [ north]
  // [-1, 0][0, 0][1, 0]  == [west][origin][east]
  //        [0,-1]                 [ south]
  // Each X is represent by an array containing its coordinates.
  // Note that you can have an arbritrary number of destinations.  For example,
  // northwest would be [-1,1].
  vector<int*> destinations;
  int north[2] = {0, 1};  destinations.push_back( north );
  int east[2]  = {1, 0};  destinations.push_back( east );
  int south[2] = {0, -1}; destinations.push_back( south );
  int west[2]  = {-1, 0}; destinations.push_back( west );
  
  // given the handle of the places, a function in the Land class,
  // and the coordinates defined above, exchange information between the 
  // places.
  land->exchangeAll( 1, Land::exchangetest_, &destinations );
  
  // ask every place to report its current status.
  land->callAll( Land::checkInMessage_ );
  
  
  
   /*  THIS SECTION OF CODE INTRODUCES AGENTS  */
  
  
  
  // Create the agents.
  // Arguments are, in order:
  //    handle, className, *argument, argument_size, *places,  initPopulation
  Agents *nomad = new Agents( 2, "Nomad", msg, 7, land, 10000 );
  
  // Perform the Agent's callAll with the 
  // agent implementation's function, msg, and message size.
  // In this case it is an initialization routine.
  nomad->callAll( Nomad::agentInit_, msg, 5 );

  //Test callAll second time, this time on the Nomads, which are agents.
  msg = (char *)("Second attempt\0");
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
  nomad->callAll( Nomad::createChild_ ); // spawn new Nomads from old ones
  nomad->callAll( Nomad::killMe_ );      // kill some nomads
  nomad->callAll( Nomad::move_ );        // move some nomads around
  
  // updates the global status of nomads so nomads have current information
  // about the population after the above changes.
  nomad->manageAll( );
  if ( printOutput == true )
    cout << "first manageAll done" << endl;

  //Test callAll with return values
  int agent_callargs2[15000];
  for ( int i = 0; i < 15000; i++ )
    agent_callargs2[i] = -i;

  if ( printOutput == true )
    cout << "nomad->callAll( callalltest ) starts" << endl;

  // callAll and save the return values from the nomads.
  retvals = (double *)
    nomad->callAll( Nomad::callalltest_, (void *)agent_callargs2,
		    sizeof( int ), sizeof( double ) );

  // print out the return values
  if(printOutput == true){
      for ( int i = 0; i < 15000; i++ )
        cout << retvals[i] << endl;
  }
  delete retvals;

  // More agent calculations followed by a manageAll to update them.
  nomad->callAll( Nomad::addData_ );
  nomad->callAll( Nomad::move2_ );
  nomad->manageAll( );

  //Test exchangeBoundary
  land->callAll( Land::printOutMessage_ );
  land->exchangeBoundary( );
  land->callAll( Land::printShadow_ );

  MASS::finish( );
}
