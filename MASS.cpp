#include <iostream> // cerr
#include <string>   // string
#include <fstream>  // ifstream
#include <netdb.h>  // gethostbyname
#include <unistd.h> // sleep
#include <sstream>  // ostringstream
#include "MASS.h"

using namespace std;

// Allocate static space
Utilities MASS::util;
vector<MNode*> MASS::mNodes;

void MASS::init( char *args[], int nProc, int nThr ) {
  vector<string> hosts;

  // variable assignment
  char *username = args[0];
  char *password = args[1];
  char *machineFilePath = args[2];
  int port = atoi( args[3] );

  // Load any *.so dynamic linking files
  // TODO

  // Read a given machine file
  ifstream machinefile( machineFilePath, std::ifstream::in  );
  if ( machinefile.is_open( ) ) {
    while ( machinefile.good( ) ) {
      string machineName;
      getline( machinefile, machineName );
      if ( machineName.size( ) > 0 ) // skip a null string at the end
	hosts.push_back( machineName ); 
    }
    machinefile.close( );
  }
  else {
    cerr << "machine file: " << machineFilePath << " could not open." << endl;
    exit( -1 );
  }

  // For debugging
  for ( int i = 0; i < int( hosts.size( ) ); i++ )
    cerr << "rank " << (i + 1) << ": " << hosts[i]  << endl;

  // Handle nProc
  if ( nProc < 0 || nProc > int( hosts.size( ) ) ) 
    nProc = hosts.size( ) + 1; // count the master node
  systemSize = nProc;

  // Initialize MASS_base.constants and identify the CWD.
  initMASS_base( "localhost", 0, nProc, port );

  // For debugging
  cerr << "CUR_DIR = " << CUR_DIR << endl;

  // Launch remote processes
  int pid = 1; // a slave process id

  for ( int i = 0; i < int( hosts.size( ) ); i++, pid++ ) {
    // retrieve each canonical remtoe machine name
    string currHostName = hosts[i];
    struct hostent *host = gethostbyname( currHostName.c_str( ) );
    if ( host == NULL ) {
      cerr << "wrong host name: " << currHostName << endl;
      exit( -1 );
    }
    currHostName = host->h_name;

    // For debugging
    cerr << "curHostName = " << currHostName << endl;
  
    // Establish an ssh2 channel to a given port
    Ssh2Connection *ssh2connection
      = util.establishConnection( currHostName.c_str( ), LIBSSH2_PORT, 
				  username, password );

    // Start a remote process
    string command = CUR_DIR;    // the absolute path to the command
    command += "/mprocess ";        // the command
    command += CUR_DIR; command += " ";      // 1st arg: current working dir
    command += currHostName; command += " "; // 2nd arg: hostName
    ostringstream convert;
    convert << pid << " "                    // 3rd arg: pid
	    << nProc << " "                  // 4th arg: #processes
	    << nThr << " "                   // 5th arg: #threads 
	    << MASS_PORT;                    // 6yh arg: MASS_PORT
    command += convert.str( );
 
    if ( ssh2connection != NULL ) {
      if ( !util.launchRemoteProcess( ssh2connection, command.c_str( ) ) ) {
	// connection failure
	util.shutdown( ssh2connection, "abnormal" );
	exit( -1 );
      }
      // A new remote process launched. The corresponding MNode created
      mNodes.push_back( new MNode( currHostName, pid, ssh2connection ) );
    }
  }

  initializeThreads( nThr );
  INITIALIZED = true;

  // Synchronize with all slave processes
  for ( int i = 0; i < int( hosts.size( ) ); i++ ) {
    cerr << "init: wait for ack from " << mNodes[i]->getHostName( ) << endl;
    Message *m = mNodes[i]->receiveMessage( );
    if ( m->getAction( ) != Message::ACK ) {
      cerr << "init didn't receive ack from rank " << ( i + 1 ) 
	   << " at " << mNodes[i]->getHostName( ) << endl;
      exit( -1 );
    }
    delete m;
  }
  cerr << "MASS::init( ) completed" << endl;
}

void MASS::finish( ) {
  cerr << "MASS::finish: started" << endl;
  Mthread::resumeThreads( Mthread::STATUS_TERMINATE );
  Mthread::barrierThreads( 0 );
  cerr << "all threads terminated" << endl;

  // Close connection and finish each mprocess
  for ( int i = 0; i < int( mNodes.size( ) ); i++ ) {
    // Send a finish messages
    cerr << "i = " << i << endl;
    Message *m = new Message( Message::FINISH );
    cerr << "message created" << endl;
    mNodes[i]->sendMessage( m );
    cerr << "FINISH sent to " << i << endl;
    delete m;
    
  }
  // Synchronize with all slaves
  barrier_all_slaves( );
}

void MASS::barrier_all_slaves( char *return_values, int stripe, int arg_size,
			       int localAgents[] ){
  // Synchronize with all slave processes                                     
  for ( int i = 0; i < int( mNodes.size( ) ); i++ ) {
    cerr << "barrier waits for ack from " << mNodes[i]->getHostName( ) << endl;
    Message *m = mNodes[i]->receiveMessage( );
    if ( m->getAction( ) != Message::ACK ) {
      cerr << "barrier didn't receive ack from rank " << ( i + 1 )
	   << " at " << mNodes[i]->getHostName( ) 
	   << " message action type = " << m->getAction( )
	   << endl;
      exit( -1 );
    }
    cerr << "localAgents[" << (i + 1) << "] = m->getAgentPopulation: "
	 << m->getAgentPopulation( ) << endl;
    if ( localAgents != NULL ) localAgents[i + 1] = m->getAgentPopulation( );
    if ( return_values != NULL && arg_size > 0 ) {
      m->getArgument( return_values + arg_size * stripe * ( i + 1 ) );
    }

    cerr << "message deleted" << endl;
    delete m;
  }
}
