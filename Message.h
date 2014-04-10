#ifndef MESSAGE_H
#define MESSAGE_H
#define VOID_HANDLE -1

#include <string>
#include <vector>
#include "RemoteExchangeRequest.h"

using namespace std;

class Message {
 public:
  enum ACTION_TYPE { EMPTY,                                    // 0
		     FINISH,                                   // 1
		     ACK,                                      // 2

		     PLACES_INITIALIZE,                        // 3
		     PLACES_CALL_ALL_VOID_OBJECT,              // 4
		     PLACES_CALL_ALL_RETURN_OBJECT,            // 5
		     PLACES_CALL_SOME_VOID_OBJECT,
		     PLACES_EXCHANGE_ALL,                      // 7
		     PLACES_EXCHANGE_ALL_REMOTE_REQUEST,       // 8
		     PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT, // 9
		     
		     AGENTS_INITIALIZE,                        // 10
		     AGENTS_CALL_ALL_VOID_OBJECT,              // 11
		     AGENTS_CALL_ALL_RETURN_OBJECT,            // 12
		     AGENTS_MANAGE_ALL 
  };

  // PLACES_INITIALIZE
  Message( ACTION_TYPE action,
	   vector<int> *size, int handle,  string classname, void *argument, 
	   int arg_size, vector<string> *hosts ) :
    action( action ), size( size ), 
    handle( handle ), dest_handle( VOID_HANDLE ), 
    functionId( 0 ), classname( classname ), 
    argument( argument ),  argument_size( arg_size ), return_size( 0 ),
    argument_in_heap( false ), hosts( hosts ), destinations( NULL ),
    dimension( 0 ), agent_population( -1 ), 
    exchangeReqList( NULL ) { };

  // PLACES_CALL_ALL_VOID_OBJECT,
  // PLACES_CALL_ALL_RETURN_OBJECT,
  // AGENTS_CALL_ALL_VOID_OBJECT,
  // AGENTS_CALL_ALL_RETURN_OBJECT
  Message( ACTION_TYPE action,
	   int handle, int functionId, void *argument, int arg_size,
	   int ret_size ) :
    action( action ), size( 0 ), 
    handle( handle ), dest_handle( VOID_HANDLE ), 
    functionId( functionId ), classname( "" ), 
    argument( argument ), argument_size( arg_size ), return_size( ret_size ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( -1 ), 
    exchangeReqList( NULL ) { };

  // PLACES_EXCHANGE_ALL
  Message( ACTION_TYPE action,
	   int handle, int dest_handle, int functionId, 
	   vector<int*> *destinations, int dimension ) :
    action( action ), size( 0 ), 
    handle( handle ), dest_handle( dest_handle ),
    functionId( functionId ), classname( "" ),
    argument( NULL ), argument_size( 0 ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( destinations ),
    dimension( dimension ), agent_population( -1 ),
    exchangeReqList( NULL ) { };

  // PLACES_EXCHANGE_ALL_REMOTE_REQUEST
  Message( ACTION_TYPE action,  
	   int handle, int dest_handle, int functionId,
           vector<RemoteExchangeRequest*> *exchangeReqList ) :
    action( action ), size( 0 ),
    handle( handle ), dest_handle( dest_handle ),
    functionId( functionId ), classname( "" ),
    argument( NULL ), argument_size( 0 ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( -1 ), 
    exchangeReqList( exchangeReqList ) { }

  // PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT
  Message( ACTION_TYPE action, char *retVals, int retValsSize ) :
    action( action ), size( 0 ),
    handle( VOID_HANDLE ), dest_handle( VOID_HANDLE ),
    functionId( 0 ), classname( "" ),
    argument( retVals ), argument_size( retValsSize ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( -1 ), 
    exchangeReqList( NULL ) { }

  // AGENTS_INITIALIZE
  Message( ACTION_TYPE action, int initPopulation, int handle,
	   int placeHandle, string className, void *argument, 
	   int argument_size ) :
    action( action ), size( 0 ),
    handle( handle ), dest_handle( placeHandle ),
    functionId( 0 ), classname( className ),
    argument( argument ), argument_size( argument_size ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( initPopulation ),
    exchangeReqList( NULL ) { };

  // FINISH
  // ACK
  Message( ACTION_TYPE action ) : 
    action( action ), size( NULL ), 
    handle( VOID_HANDLE ), dest_handle( VOID_HANDLE ),
    functionId( 0 ), classname( "" ), 
    argument( NULL ),  argument_size( 0 ),  return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( -1 ),
    exchangeReqList( NULL ) {  };

  // ACK used for PLACES_CALL_ALL_RETURN_OBJECT
  Message( ACTION_TYPE action, void *argument, int arg_size ) :
    action( action ), size( NULL ), 
    handle( VOID_HANDLE ), dest_handle( VOID_HANDLE ),
    functionId( 0 ), classname( "" ), 
    argument( argument ), argument_size( arg_size ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( -1 ),
    exchangeReqList( NULL ) { };

  // ACK used for AGENTS_INITIALIZE nad AGENTS_CALL_ALL_VOID_OBJECT
  Message( ACTION_TYPE action, int localPopulation ) :
    action( action ), size( NULL ), 
    handle( VOID_HANDLE ), dest_handle( VOID_HANDLE ),
    functionId( 0 ), classname( "" ), 
    argument( NULL ), argument_size( 0 ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ), destinations( NULL ),
    dimension( 0 ), agent_population( localPopulation ),
    exchangeReqList( NULL ) { };

  // EMPTY
  Message( ) : 
    action( EMPTY ), size( NULL ), 
    handle( VOID_HANDLE ), dest_handle( VOID_HANDLE ), 
    functionId( 0 ), classname( "" ), 
    argument( NULL ),  argument_size( 0 ), return_size( 0 ),
    argument_in_heap( false ), hosts( NULL ) , destinations( NULL ),
    dimension( 0 ), agent_population( -1 ),
    exchangeReqList( NULL ) {  };

  ~Message( ); // delete argument and hosts.

  char *serialize( int &size );
  void deserialize( char *msg, int size );
  
  ACTION_TYPE getAction( ) { return action; };
  vector<int> getSize( ) { return *size; };
  int getHandle( ) { return handle; };
  int getDestHandle( ) { return dest_handle; };
  int getFunctionId( ) { return functionId; };
  string getClassname( ) { return classname; };
  bool isArgumentValid( ) { return ( argument != NULL ); };
  void getArgument( void *arg ) { memcpy( arg, argument, argument_size ); };
  int getArgumentSize( ) { return argument_size; };
  int getReturnSize( ) { return return_size; };
  int getAgentPopulation( ) { return agent_population; };
  vector<string> getHosts( ) { return *hosts; };
  vector<int*> *getDestinations( ) { return destinations; };
  vector<RemoteExchangeRequest*> *getExchangeReqList( ) 
    { return exchangeReqList; };

 protected:
  ACTION_TYPE action;
  vector<int> *size;
  int handle;
  int dest_handle;
  int functionId;
  string classname;      // classname.so must be located in CWD.
  void *argument;
  int argument_size;
  int return_size;
  bool argument_in_heap;
  vector<string> *hosts; // all hosts participated in computation
  vector<int*> *destinations; // all destinations of exchangeAll
  int dimension;
  int agent_population;
  vector<RemoteExchangeRequest*> *exchangeReqList;
};

#endif
