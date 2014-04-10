#include <sstream>      // ostringstream
#include "MASS_base.h"  // MASS_base::log( )
#include "Places_base.h"
#include "Agents_base.h"
#include "Mthread.h"

pthread_mutex_t Mthread::lock;
pthread_cond_t Mthread::barrier_ready;
pthread_cond_t Mthread::barrier_finished;
int Mthread::barrier_count;
Mthread::STATUS_TYPE Mthread::status;
int Mthread::threadCreated;
int Mthread::agentBagSize;

void Mthread::init( ) {
  pthread_mutex_init( &lock, NULL ); 
  pthread_cond_init( &barrier_ready, NULL );
  pthread_cond_init( &barrier_finished, NULL );
  status = STATUS_READY;
  barrier_count = 0;
}

void *Mthread::run( void *param ) {
  int tid = *(int *)param;
  threadCreated = tid;

  // breath message
  ostringstream convert;
  convert << "Mthread[" << tid << "] inovked";
  MASS_base::log( convert.str( ) );

  // the followign variables are used to call callAll( )
  Places_base *places = NULL;
  Places_base *destinationPlaces = NULL;

//Chris
  Agents_base *agents = NULL;


  int functionId = 0;
  void *argument = NULL;
  int arg_size = 0;
  int ret_size = 0;
  Message::ACTION_TYPE msgType = Message::EMPTY;
  vector<int*> *destinations = NULL;

  bool running = true;
  while ( running ) {
    // wait for a new command
    pthread_mutex_lock( &lock );
    if ( status == STATUS_READY )
      pthread_cond_wait( &barrier_ready, &lock );

    // wake-up message
    convert.str( "" );
    convert << "Mthread[" << tid << "] woken up";
    MASS_base::log( convert.str( ) );

    pthread_mutex_unlock( &lock );

    // perform each task
    switch( status ) {
    case STATUS_READY:
      MASS_base::log( "Mthread reached STATUS_READY in switch" );
      exit( -1 );
      break;
    case STATUS_TERMINATE:
      running = false;
      break;
    case STATUS_CALLALL:
      places = MASS_base::getCurrentPlaces( );
      functionId = MASS_base::getCurrentFunctionId( );
      argument = MASS_base::getCurrentArgument( );
      arg_size = MASS_base::getCurrentArgSize( );
      msgType = MASS_base::getCurrentMsgType( );
      ret_size = MASS_base::getCurrentRetSize( );

      convert.str( "" );
      convert << "Mthread[" << tid << "] works on CALLALL:"
	      << " placese = " << (void *)places
	      << " functionId = " << functionId
	      << " argument = " << argument
	      << " arg_size = " << arg_size
	      << " msgType = " << msgType
	      << " ret_size = " << ret_size;
      MASS_base::log( convert.str( ) );

      if ( msgType == Message::PLACES_CALL_ALL_VOID_OBJECT ) {
	//cerr << "Mthread[" << tid << "] call all void object" << endl;
	places->callAll( functionId, argument, tid );
      }
      else {
	//cerr << "Mthread[" << tid << "] call all return object" << endl;
	places->callAll( functionId, argument, arg_size, ret_size, tid );
      }
      break;

    case STATUS_EXCHANGEALL:
      convert.str( "" );
      convert << "Mthread[" << tid << "] works on EXCHANGEALL";
      MASS_base::log( convert.str( ) );

      places = MASS_base::getCurrentPlaces( );
      functionId = MASS_base::getCurrentFunctionId( );
      destinationPlaces = MASS_base::getDestinationPlaces( );
      destinations = MASS_base::getCurrentDestinations( );

      places->exchangeAll( destinationPlaces, functionId, destinations, tid );
      break;

    case STATUS_AGENTSCALLALL:
	agents = MASS_base::getCurrentAgents();
	functionId = MASS_base::getCurrentFunctionId();
	argument = MASS_base::getCurrentArgument();
	arg_size = MASS_base::getCurrentArgSize();
	msgType = MASS_base::getCurrentMsgType();
	ret_size = MASS_base::getCurrentRetSize();

	convert.str("");
	convert << "Mthread[" << tid << "] works on AGENST_CALLALL:"
		<< " agents = " << (void*)agents
		<< " functionId = " << functionId
		<< " argument = " << argument
		<< " arg_size = " << arg_size
		<< " msgType = " << msgType
		<< " ret_size = " << ret_size;
	MASS_base::log( convert.str() );

	if(msgType == Message::AGENTS_CALL_ALL_VOID_OBJECT){
		//cerr << "Mthread[" << tid << "] call all agent void object" << endl;
		agents->callAll(functionId, argument, tid);
	}else{
		//cerr << "Mthread[" << tid << "] call all agents return object" << endl;
		agents->callAll(functionId, argument, arg_size, ret_size, tid);
	}
	break;
     //Chris ToDo: handle Manageall switch statement
      case STATUS_MANAGEALL:
		
	//Get agents to be called with Manageall
	agents = MASS_base::getCurrentAgents( );

	//Send logging message
	convert.str("");
	convert << "Mthread[" << tid << "] works on MANAGEALL:"
		<< " agents = " << (void*)agents;
	MASS_base::log( convert.str() );

	//Sent message for manageall
	agents->manageAll( tid );
	
	break;
    }

    // barrier
    barrierThreads( tid );
      
  }

  // last message
  convert.str( "" );
  convert << "Mthread[" << tid << "] terminated";
  MASS_base::log( convert.str( ) );

  return NULL;
}

void Mthread::resumeThreads( STATUS_TYPE new_status ) {
  pthread_mutex_lock( &lock );
  status = new_status;
  pthread_cond_broadcast( &barrier_ready );
  pthread_mutex_unlock( &lock );
}

void Mthread::barrierThreads( int tid ) {
  pthread_mutex_lock( &lock );
  if ( ++barrier_count < int( MASS_base::threads.size( ) ) ) {
    ostringstream convert;
    convert << "tid[" << tid << "] waiting";
    MASS_base::log( convert.str( ) );
    pthread_cond_wait( &barrier_ready, &lock );
  } else {
    barrier_count = 0;
    status = STATUS_READY;
    ostringstream convert;
    convert << "tid[" << tid << "] woke up all";
    MASS_base::log( convert.str( ) );
    pthread_cond_broadcast( &barrier_ready );
  }
  pthread_mutex_unlock( &lock );
}
