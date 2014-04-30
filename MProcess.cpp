#include <stdlib.h> // atoi
#include <unistd.h> // chdir, sleep (just for debugging)
#include <sys/stat.h> // mkdir
#include <sys/types.h> // mkdir, opendir, closedir
#include <dirent.h>    // opendir, closedir
#include <sstream>     // ostringstream
#include "MProcess.h"
#include "MASS_base.h"
#include "Mthread.h"

MProcess::MProcess( char *name, int myPid, int nProc, int nThr, int port ) {
  this->hostName = new string( name );
  this->myPid = myPid;
  this->nProc = nProc;
  this->nThr = nThr;
  MASS_base::initMASS_base( name, myPid, nProc, port );
  
  // Create a logger
  DIR *dir = NULL;
  if ( ( dir =  opendir( MASS_LOGS ) ) == NULL ) 
    mkdir( MASS_LOGS, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
  else
    closedir( dir );

  MASS_base::initializeThreads( nThr );
}

void MProcess::start( ) {
  MASS_base::log( "started" );

  // Synchronize with the master node first.
  sendAck( );

  bool alive = true;
  while( alive ) {
    // receive a new message from the master
    Message *m = receiveMessage( );

    ostringstream convert;
    convert << "A new message received: action = " << m->getAction( ) << endl;
    MASS_base::log( convert.str( ) );
    int *data; // debug

    // get prepared for the following arguments for PLACES_INITIALIZE
    vector<int> size;            // size[]
    vector<string> hosts; hosts.clear( ); 
    int argument_size = 0;       // argument[argument_size];
    char *argument = NULL;
    Places_base *places = NULL;  // new Places
    Agents_base *agents = NULL;  // new Agents

    // retrieve an argument
    argument_size = m->getArgumentSize( );
    argument = ( argument_size > 0 ) ? new char[argument_size] : NULL;
    m->getArgument( argument );

    if ( m != NULL ) {
      switch( m->getAction( ) ) {
      case Message::ACK:
	sendAck( );
	break;

      case Message::EMPTY:
	MASS_base::log( "EMPTY received!!!!" );
	sendAck( );
	break;

      case Message::FINISH:
	Mthread::resumeThreads( Mthread::STATUS_TERMINATE );
	Mthread::barrierThreads( 0 );
	sendAck( );
	alive = false;
	MASS_base::log( "FINISH received and ACK sent" );
	break;

      case Message::PLACES_INITIALIZE:
	MASS_base::log( "PLACES_INITIALIZE received" );
	// create a new Places
	size = m->getSize( );

	places = new Places_base( m->getHandle( ), m->getClassname( ),
				  argument, argument_size, 
				  size.size( ), &size[0] );

	for ( int i = 0; i < int( m->getHosts( ).size( ) ); i++ )
	  hosts.push_back( m->getHosts( )[i] );
	// establish all inter-node connections within setHosts( )
	MASS_base::setHosts( hosts );
	
	MASS_base::placesMap.
	  insert( map<int, Places_base*>::value_type( m->getHandle( ), 
						      places ) );
	sendAck( );
	MASS_base::log( "PLACES_INITIALIZE completed and ACK sent" );
	break;

      case Message::PLACES_CALL_ALL_VOID_OBJECT:
	MASS_base::log( "PLACES_CALL_ALL_VOID_OBJECT received" );

	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];
	MASS_base::currentFunctionId = m->getFunctionId( );
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize = argument_size;
	MASS_base::currentMsgType = m->getAction( );

	// resume threads to work on call all.
	Mthread::resumeThreads( Mthread::STATUS_CALLALL );

	// TODO: call all implementation
	// 3rd arg: 0 = the main thread id
	MASS_base::
	  currentPlaces->callAll( m->getFunctionId( ), (void *)argument, 0 );
	
	// resume threads to work on call all.
	Mthread::barrierThreads( 0 );
	
	sendAck( );
	break;

      case Message::PLACES_CALL_ALL_RETURN_OBJECT:
	MASS_base::log( "PLACES_CALL_ALL_RETURN_OBJECT received" );
	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];
	MASS_base::currentFunctionId = m->getFunctionId( );
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize 
	  = argument_size / MASS_base::currentPlaces->places_size;
	MASS_base::currentRetSize = m->getReturnSize( );
	MASS_base::currentMsgType = m->getAction( );
	MASS_base::currentReturns 
	  = new char[MASS_base::currentPlaces->places_size 
		     * MASS_base::currentRetSize];

	data = (int *)argument;
	for ( int i = 0; i < 2500; i++ ) {
	  convert.str( "" );
	  convert << *(data + i);
	  MASS_base::log( convert.str( ) );
	}

	// resume threads to work on call all.
	Mthread::resumeThreads( Mthread::STATUS_CALLALL );

	// TODO: call all implementation
	// 3rd arg: 0 = the main thread id

	MASS_base::
	  currentPlaces->callAll( MASS_base::currentFunctionId,
				  MASS_base::currentArgument,
				  MASS_base::currentArgSize,
				  MASS_base::currentRetSize,
				  0 );

	// confirm all threads are done with exchangeall.
	Mthread::barrierThreads( 0 );
	sendReturnValues( (void *)MASS_base::currentReturns, 
			  MASS_base::currentPlaces->places_size,
			  MASS_base::currentRetSize );
	delete MASS_base::currentReturns;
	break;

      case Message::PLACES_CALL_SOME_VOID_OBJECT:
	break;

      case Message::PLACES_EXCHANGE_ALL:
	convert.str( "" );
	convert << "PLACES_EXCHANGE_ALL recweived handle = " 
		<< m->getHandle( ) << " dest_handle = " << m->getDestHandle();
	MASS_base::log( convert.str( ) );

	// retrieve the corresponding places
	MASS_base::currentPlaces = MASS_base::placesMap[ m->getHandle( ) ];
	MASS_base::destinationPlaces =MASS_base::placesMap[m->getDestHandle()];
	MASS_base::currentFunctionId = m->getFunctionId( );
	MASS_base::currentDestinations = m->getDestinations( );

	// reset requestCounter by the main thread
	MASS_base::requestCounter = 0;

	// for debug
	MASS_base::showHosts( );

	// resume threads to work on call all.
	Mthread::resumeThreads( Mthread::STATUS_EXCHANGEALL );	

	// exchangeall implementation
	MASS_base::
	  currentPlaces->exchangeAll( MASS_base::destinationPlaces,
				      MASS_base::currentFunctionId, 
				      MASS_base::currentDestinations, 0 );

	// confirm all threads are done with exchangeall.
	Mthread::barrierThreads( 0 );
	MASS_base::log( "barrier done" );

	sendAck( );
	MASS_base::log( "PLACES_EXCHANGE_ALL completed and ACK sent" );
	break;

      case Message::PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
      case Message::PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:
	break;

      case Message::AGENTS_INITIALIZE:
	MASS_base::log( "AGENTS_INITIALIZE received" );

	agents = new Agents_base( m->getHandle( ), m->getClassname( ),
				  argument, argument_size,
				  m->getDestHandle( ), 
				  m->getAgentPopulation( ) );
	
	MASS_base::agentsMap.
	  insert( map<int, Agents_base*>::value_type( m->getHandle( ), 
						      agents ) );
	sendAck( agents->localPopulation );
	MASS_base::log( "AGENTS_INITIALIZE completed and ACK sent" );
	break;

      case Message::AGENTS_CALL_ALL_VOID_OBJECT:

	//Chris ToDo: Implement
	MASS_base::log("AGENTS_CALL_ALL_VOID_OBJECT received" );
	MASS_base::currentAgents = MASS_base::agentsMap[m->getHandle() ];
	MASS_base::currentFunctionId = m->getFunctionId();
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize = argument_size;
	MASS_base::currentMsgType = m->getAction();
	
	Mthread::agentBagSize = MASS_base::dllMap[m->getHandle()]->agents->size();

	Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL );

	MASS_base::currentAgents->callAll(m->getFunctionId(), (void *)argument, 0);

	Mthread::barrierThreads(0);

	sendAck();
	break;

      case Message::AGENTS_CALL_ALL_RETURN_OBJECT:

	//Chris ToDo: Replace currentAgents->agents_size
	
	MASS_base::log("AGENTS_CALL_ALL_RETURN_OBJECT received");
	MASS_base::currentAgents = MASS_base::agentsMap[m->getHandle()];
	MASS_base::currentFunctionId = m->getFunctionId();
	MASS_base::currentArgument = (void *)argument;
	MASS_base::currentArgSize = argument_size / 
	MASS_base::currentAgents->localPopulation;
	MASS_base::currentRetSize = m->getReturnSize();
	MASS_base::currentMsgType = m->getAction();
	MASS_base::currentReturns = new char[MASS_base::currentAgents->localPopulation * MASS_base::currentRetSize];
	
	//Debugging code
	data = (int *)argument;
	for( int i = 0; i < 2500; i++){
		convert.str("");
		convert << *(data + i);
		MASS_base::log(convert.str() );
	}

	Mthread::agentBagSize = MASS_base::dllMap[m->getHandle()]->agents->size();
	Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL);

	MASS_base::currentAgents->callAll( MASS_base::currentFunctionId,
						MASS_base::currentArgument,
						MASS_base::currentArgSize,
						MASS_base::currentRetSize,
						0 );

	Mthread::barrierThreads( 0);
	sendReturnValues( (void *)MASS_base::currentReturns,
				MASS_base::currentAgents->localPopulation,
				MASS_base::currentRetSize );

	delete MASS_base::currentReturns;
	
	break;
      case Message::AGENTS_MANAGE_ALL:

	MASS_base::log("AGENTS_MANAGE_ALL received");
	MASS_base::currentAgents = MASS_base::agentsMap[m->getHandle()];

	Mthread::agentBagSize = MASS_base::dllMap[m->getHandle()]->agents->size();
	Mthread::resumeThreads(Mthread::STATUS_MANAGEALL);

	MASS_base::currentAgents->manageAll( 0 ); // 0 = the main thread id

	Mthread::barrierThreads( 0 );
	sendAck( MASS_base::currentAgents->localPopulation );

	break;
	
      case Message:: AGENTS_MIGRATION_REMOTE_REQUEST:
	break;
	
      }
      delete m;
    }
    
  }
}

void MProcess::sendAck( ) {
  Message *msg = new Message( Message::ACK );
  sendMessage( msg );
  delete msg;
}

void MProcess::sendAck( int localPopulation ) {
  Message *msg = new Message( Message::ACK, localPopulation );
  sendMessage( msg );
  delete msg;
}

void MProcess::sendReturnValues( void *argument, int places_size, 
				int return_size ) {
  Message *msg = new Message( Message::ACK, argument, 
			      places_size * return_size );
  sendMessage( msg );
  delete msg;
}

void MProcess::sendMessage( Message *msg ) {
  int msg_size = 0;
  char *byte_msg = msg->serialize( msg_size );
  ostringstream convert;
  convert << "sendMessage size = " << msg_size << endl;
  MASS_base::log( convert.str( ) );
  write( 1, (void *)&msg_size, sizeof( int ) );  // send a message size
  write( 1, byte_msg, msg_size );                // send a message body
  fsync( 1 );
}

Message *MProcess::receiveMessage( ) {
  int size = -1;
  int nRead = 0;
  if ( read( 0, (void *)&size, sizeof( int ) ) > 0 ) {// receive a message size

    ostringstream convert;
    convert << "receiveMessage: size = " << size << endl;
    MASS_base::log( convert.str( ) );
    
    char *buf = new char[size];
    for ( nRead = 0;
	  ( nRead += read( 0, buf + nRead, size - nRead) ) < size; );
    Message *m = new Message( );
    m->deserialize( buf, size );
    return m;
  } else {
    MASS_base::log( "receiveMessage error" );
    exit( -1 );
  }
}


int main( int argc, char* argv[] ) {
  // receive all arguments
  char *cur_dir = argv[1];
  char *hostName = argv[2];
  int myPid = atoi( argv[3] );
  int nProc = atoi( argv[4] );
  int nThr = atoi( argv[5] );
  int port = atoi( argv[6] );

  // set the current working directory to where the master node is running.
  chdir( cur_dir );

  // launch an MProcess at each slave node.
  MProcess process( hostName, myPid, nProc, nThr, port );
  process.start( );
}

