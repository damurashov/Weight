#include "Places.h"
#include "MASS.h"
#include "Message.h"
#include "Mthread.h"
#include <iostream>
#include <dlfcn.h> // dlopen, dlsym, and dlclose

//Used to toggle comments from Places.cpp
//const bool printOutput = false;
const bool printOutput = true;

/**
 * Creates a Places Object that serves as a container Object for the
 * neighborhood (collection) of each individual Place in a simulation, providing
 * methods/functionality that corresponds with the entire simulation space.
 *
 * Instantiates a shared array with "size[]" from the "className" class by
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 *
 * Dimensions for the simulation space are enumerated in the "..." (variable
 * argument list) format.
 *
 * @param handle          a unique identifer that designates a group of Place
 *                        Objects as all belonging to the same simulation
 *                        (Places). Must be unique over all machines.
 * @param className       name of user-created Places class to load
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 * @param dim             how many dimensions this simulation encompasses
 * @param ...             variable argument parameters, should be equal to the
 *                        size (int) of each dimension in the simulation space
 */
Places::Places( int handle, string className, void *argument, int argument_size,
    int dim, ... ) :
    Places_base( handle, className, 0, argument, argument_size, dim, NULL ) {

  size = new int[dim];
  // Extract each dimension's length
  va_list list;
  va_start( list, dim );

  for ( int i = 0; i < dim; i++ ) {
    size[i] = va_arg( list, int );
  }
  va_end( list );

  init_all( argument, argument_size ); // explicitly call Places_base.init_all
  init_master( argument, argument_size, 0 );
}

/**
 * Creates a Places Object that serves as a container Object for the
 * neighborhood (collection) of each individual Place in a simulation, providing
 * methods/functionality that corresponds with the entire simulation space.
 *
 * Instantiates a shared array with "size[]" from the "className" class by
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 *
 * Dimensions for the simulation space are ennumerated in the "..." (variable
 * argument list) format.
 *
 * @param handle          a unique identifer that designates a group of Place
 *                        Objects as all belonging to the same simulation
 *                        (Places). Must be unique over all machines.
 * @param className       name of user-created Places class to load
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 * @param dim             how many dimensions this simulation encompasses
 * @param size            array of numbers (int), representing the size of each
 *                        corresponding dimension in the simulation space
 */
Places::Places( int handle, string className, void *argument, int argument_size,
    int dim, int size[] ) :
    Places_base( handle, className, 0, argument, argument_size, dim, size ) {

  // init_all called within Places_base
  init_master( argument, argument_size, 0 );
}

/**
 * Creates a Places Object that serves as a container Object for the
 * neighborhood (collection) of each individual Place in a simulation, providing
 * methods/functionality that corresponds with the entire simulation space.
 *
 * Instantiates a shared array with "size[]" from the "className" class by
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 *
 * Dimensions for the simulation space are ennumerated in the "..." (variable
 * argument list) format.
 *
 * @param handle          a unique identifer that designates a group of Place
 *                        Objects as all belonging to the same simulation
 *                        (Places). Must be unique over all machines.
 * @param className       name of user-created Places class to load
 * @param boundary_width  width of the boundary to place between stripes
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 * @param dim             how many dimensions this simulation encompasses
 * @param ...             variable argument parameters, should be equal to the
 *                        size (int) of each dimension in the simulation space
 */
Places::Places( int handle, string className, int boundary_width,
    void *argument, int argument_size, int dim, ... ) :
    Places_base( handle, className, boundary_width, argument, argument_size,
        dim, NULL ) {

  size = new int[dim];
  // Extract each dimension's length
  va_list list;
  va_start( list, dim );

  for ( int i = 0; i < dim; i++ ) {
    size[i] = va_arg( list, int );
  }
  va_end( list );

  init_all( argument, argument_size ); // explicitly call Places_base.init_all
  init_master( argument, argument_size, boundary_width );
}

/**
 * Creates a Places Object that serves as a container Object for the
 * neighborhood (collection) of each individual Place in a simulation, providing
 * methods/functionality that corresponds with the entire simulation space.
 *
 * Instantiates a shared array with "size[]" from the "className" class by
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 *
 * Dimensions for the simulation space are ennumerated in the "..." (variable
 * argument list) format.
 *
 * @param handle          a unique identifer that designates a group of Place
 *                        Objects as all belonging to the same simulation
 *                        (Places). Must be unique over all machines.
 * @param className       name of user-created Places class to load
 * @param boundary_width  width of the boundary to place between stripes
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 * @param dim             how many dimensions this simulation encompasses
 * @param size            array of numbers (int), representing the size of each
 *                        corresponding dimension in the simulation space
 */
Places::Places( int handle, string className, int boundary_width,
    void *argument, int argument_size, int dim, int size[] ) :
    Places_base( handle, className, boundary_width, argument, argument_size,
        dim, size ) {

  // init_all called within Places_base
  init_master( argument, argument_size, boundary_width );
}

/**
 * This method creates the simulation space for Places, before sending an
 * initialize message (command) to all hosts running this simulation. This
 * effectively sets up the simulation space and initializes Places across
 * all hosts.
 *
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 * @param boundary_width  width of the boundary to place between stripes
 */
void Places::init_master( void *argument, int argument_size,
    int boundary_width ) {
  // convert size[dimension] to vector<int>
  vector<int> *size_vector = new vector<int>( dimension );
  size_vector->assign( size, size + dimension );

  // create a list of all host names;  
  // the master IP name
  char localhost[100];
  bzero( localhost, 100 );
  gethostname( localhost, 100 );
  vector<string> hosts;
  hosts.push_back( *( new string( localhost ) ) );

  // all the slave IP names
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    hosts.push_back( MASS::mNodes[i]->getHostName( ) );
  }

  // create a new list for message
  vector<string> *tmp_hosts = new vector<string>( hosts );

  Message *m = new Message( Message::PLACES_INITIALIZE, size_vector, handle,
      className, argument, argument_size, boundary_width, tmp_hosts );

  // send a PLACES_INITIALIZE message to each slave
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    MASS::mNodes[i]->sendMessage( m );

    if ( printOutput == true ) {
      cerr << "PLACES_INITIALIZE sent to " << i << endl;
    }
  }
  delete m;

  // establish all inter-node connections within setHosts( )
  MASS_base::setHosts( hosts );

  // register this places in the places hash map
  MASS_base::placesMap.insert(
      map<int, Places_base*>::value_type( handle, this ) );

  // Synchronized with all slave processes
  MASS::barrier_all_slaves( );
}

/**
 * Calls the method specified with functionId against all Place instances in the
 * simulation. Done in parallel among multi-processes/threads.
 *
 * @param functionId  user-defined ID (handle) for corresponding function to
 *                    call in their user-defined Place class
 */
void Places::callAll( int functionId ) {
  ca_setup( functionId, NULL, 0, 0, Message::PLACES_CALL_ALL_VOID_OBJECT );
}

/**
 * Calls the method specified with functionId against all Place instances in the
 * simulation - passing in the argument referenced (with size equal to arg_size)
 * to the method. Done in parallel among multi-processes/threads.
 *
 * @param functionId  user-defined ID (handle) for corresponding function to
 *                    call in their user-defined Place class
 * @param argument    argument to pass into each Place method call
 * @param arg_size    total size of the argument
 */
void Places::callAll( int functionId, void *argument, int arg_size ) {
  if ( printOutput == true ) {
    cerr << "callAll void object" << endl;
  }

  ca_setup( functionId, argument, arg_size, 0, // ret_size = 0
      Message::PLACES_CALL_ALL_VOID_OBJECT );
}

/**
 * Calls the method specified with functionId of all array elements as
 * passing arguments[i] to element[i]'s method, and receives a return
 * value from it into (void *)[i] whose element's size is return_size. Done
 * in parallel among multi-processes/threads. In case of a multi-
 * dimensional array, "i" is considered as the index when the array is
 * flattened to a single dimension.
 *
 * @param functionId  user-defined ID (handle) for corresponding function to
 *                    call in their user-defined Place class
 * @param argument    argument to pass into each Place method call
 * @param arg_size    total size of the argument
 * @param ret_size    size of each return value
 * @return            reference to (pointer) the location of the message queue,
 *                    which contains results from each call (size is equal to
 *                    ret_size argument - so, you can traverse using pointer
 *                    arithmetic)
 */
void *Places::callAll( int functionId, void *argument[], int arg_size,
    int ret_size ) {

  if ( printOutput == true ) {
    cerr << "callAll return object" << endl;
  }

  return ca_setup( functionId, (void *) argument, arg_size, ret_size,
      Message::PLACES_CALL_ALL_RETURN_OBJECT );
}

/**
 * This method helps reduce repeated code by providing a single interface for
 * every callAll() method in Places to utilize - handling the actual
 * construction/sending of the Message, synchronization, and barrier needed to
 * ensure that slaves have completed and return values (if needed) are available
 * for calling methods to return.
 * 
 * @param functionId  user-defined ID (handle) for corresponding function to
 *                    call in their user-defined Place class
 * @param argument    argument to pass into each Place method call
 * @param arg_size    total size of the argument
 * @param ret_size    size of each return value
 * @param type        type of Message (action) to send
 * @return            reference to (pointer) the location of the message queue,
 *                    which contains results from each call (size is equal to
 *                    ret_size argument - so, you can traverse using pointer
 *                    arithmetic)
 */
void *Places::ca_setup( int functionId, void *argument, int arg_size,
    int ret_size, Message::ACTION_TYPE type ) {
  // calculate the total argument size for return-objects
  int total = 1; // the total number of place elements
  for ( int i = 0; i < dimension; i++ )
    total *= size[i];
  int stripe = total / MASS_base::systemSize;

  // send a PLACES_CALLALL message to each slave
  Message *m = NULL;
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    // create a message
    if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT )
      m = new Message( type, this->handle, functionId, argument, arg_size,
          ret_size );
    else { // PLACES_CALL_ALL_RETURN_OBJECT
      m = new Message( type, this->handle, functionId,
      // argument body
          (char *) argument + arg_size * stripe * ( i + 1 ),
          // argument size
          ( i == int( MASS::mNodes.size( ) ) - 1 ) ?
              arg_size * ( total - stripe * ( i + 1 ) ) : // + rmdr
              arg_size * stripe, ret_size ); // no remainder

      if ( printOutput == true ) {
        cerr << "Places.callAll: arg_size = " << arg_size << " stripe = "
            << stripe << " i + 1 = " << ( i + 1 ) << endl;
      }
      /*
       int *data = (int *)((char *)argument + arg_size * stripe * ( i + 1 ));
       for ( int i = 0; i < stripe; i++ )
       cerr << *(data + i) << endl;
       */
    }

    // send it
    MASS::mNodes[i]->sendMessage( m );

    if ( printOutput == true ) {
      cerr << "PLACES_CALL_ALL " << m->getAction( ) << " sent to " << i << endl;
    }

    // make sure to delete it
    delete m;
  }

  // retrieve the corresponding places
  MASS_base::currentPlaces = this;
  MASS_base::currentFunctionId = functionId;
  MASS_base::currentArgument = argument;
  MASS_base::currentArgSize = arg_size;
  MASS_base::currentMsgType = type;
  MASS_base::currentRetSize = ret_size;
  MASS_base::currentReturns =
      ( type == Message::PLACES_CALL_ALL_VOID_OBJECT ) ?
          NULL : new char[total * MASS_base::currentRetSize]; // prepare an entire return space

  // resume threads
  Mthread::resumeThreads( Mthread::STATUS_CALLALL );

  // callall implementation
  if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT )
    Places_base::callAll( functionId, argument, 0 ); // 0 = the main thread id
  else
    Places_base::callAll( functionId, (void *) argument, arg_size, ret_size,
        0 );

  // confirm all threads are done with callAll.
  Mthread::barrierThreads( 0 );

  // Synchronized with all slave processes
  if ( type == Message::PLACES_CALL_ALL_RETURN_OBJECT )
    MASS::barrier_all_slaves( MASS_base::currentReturns, stripe,
        MASS_base::currentRetSize );
  else
    MASS::barrier_all_slaves( );

  return (void *) MASS_base::currentReturns;
}

/**
 * This method calls the function specified with functionId of one or more
 * selected array elements - without passing any argument value to the indicated
 * method. If arguments need to be passed to the referenced function, please use
 * the following method instead:
 *
 * void Places::callSome( int functionId, void *arguments[], int arg_size,
 * int dim, int index[] )
 *
 * No associated return values are stored as a result of these calls. If return
 * values are needed, please use the following method instead:
 *
 * void *Places::callSome( int functionId, void *arguments[], int arg_size,
 * int ret_size, int dim, int index[] )
 *
 * If index[i] is a non-negative number, it references a particular element
 * within a row or a column. If index[i] is a negative number, say –x, it
 * instead refers to all elements in the row or column x. In the case of a
 * multidimensional array, “i” should be thought of as the index when the array
 * is flattened to a single dimension.
 *
 * Computation is performed in parallel among multi-processes/threads.
 *
 * @param functionId  ID (int) of the function to call at each place
 * @param dim         dimension of the array to call elements from. While
 *                    dimensions are typically considered in the form x, y, or
 *                    z, this value is expressed numerically (int). As such,
 *                    you can think of 0 as equivalent to the 'x' axis, 1 being
 *                    equivalent to 'y', and so on...
 * @param index       array of index values (int) that refer to the specific
 *                    element, numbered according to the dimension. Using this
 *                    model, index 0 will always refer to the same actual
 *                    element (Place). However, an index of 1 will refer to the
 *                    first element along the dimension specified by 'dim'
 * @param indexSize   this is the physical number of elements that are being
 *                    referenced within the index[] array. This value is
 *                    necessary since there is no method to determine the size
 *                    of an array at runtime
 */
void Places::callSome( int functionId, int dim, int index[], int indexSize ) {
  if ( printOutput == true ) {
    cerr << "Places::callSome( int functionId, int dim, int index[], int "
        "indexSize ) reached" << endl;
  }

  cs_setup( functionId, NULL, 0, 0, dim, index, indexSize,
      Message::PLACES_CALL_SOME_VOID_OBJECT );
}

/**
 * This method calls the function specified with functionId of one or more
 * selected array elements - passing arguments[i] to element[i]’s method.
 *
 * No associated return values are stored as a result of these calls. If return
 * values are needed, please use the following method instead:
 *
 * void *Places::callSome( int functionId, void *arguments[], int arg_size,
 * int ret_size, int dim, int index[] )
 *
 * If index[i] is a non-negative number, it references a particular element
 * within a row or a column. If index[i] is a negative number, say –x, it
 * instead refers to all elements in the row or column x. In the case of a
 * multidimensional array, “i” should be thought of as the index when the array
 * is flattened to a single dimension.
 *
 * Computation is performed in parallel among multi-processes/threads.
 *
 * @param functionId  ID (int) of the function to call at each place
 * @param arguments   list of arguments to send to each place
 * @param arg_size    size of each argument
 * @param dim         dimension of the array to call elements from. While
 *                    dimensions are typically considered in the form x, y, or
 *                    z, this value is expressed numerically (int). As such,
 *                    you can think of 0 as equivalent to the 'x' axis, 1 being
 *                    equivalent to 'y', and so on...
 * @param index       array of index values (int) that refer to the specific
 *                    element, numbered according to the dimension. Using this
 *                    model, index 0 will always refer to the same actual
 *                    element (Place). However, an index of 1 will refer to the
 *                    first element along the dimension specified by 'dim'
 * @param indexSize   this is the physical number of elements that are being
 *                    referenced within the index[] array. This value is
 *                    necessary since there is no method to determine the size
 *                    of an array at runtime
 */
void Places::callSome( int functionId, void *arguments[], int arg_size, int dim,
    int index[], int indexSize ) {
  if ( printOutput == true ) {
    cerr << "Places::callSome( int functionId, void *arguments[], "
        "int arg_size, int dim, int index[], int indexSize ) reached" << endl;
  }

  cs_setup( functionId, (void *) arguments, arg_size, 0, dim, index, indexSize,
      Message::PLACES_CALL_SOME_VOID_OBJECT );
}

/**
 * This method calls the function specified with functionId of one or more
 * selected array elements - passing arguments[i] to element[i]’s method. It
 * then receives a return value from the call and stores this value in an array
 * of void pointers at element i ( (void *)[i] ) - the size of this value is
 * set and must be equal to the return_size argument.
 *
 * If index[i] is a non-negative number, it references a particular element
 * within a row or a column. If index[i] is a negative number, say –x, it
 * instead refers to all elements in the row or column x. In the case of a
 * multidimensional array, “i” should be thought of as the index when the array
 * is flattened to a single dimension.
 *
 * Computation is performed in parallel among multi-processes/threads.
 *
 * @param functionId  ID (int) of the function to call at each place
 * @param arguments   list of arguments to send to each place
 * @param arg_size    size of each argument
 * @param ret_size    size of each return value
 * @param dim         dimension of the array to call elements from. While
 *                    dimensions are typically considered in the form x, y, or
 *                    z, this value is expressed numerically (int). As such,
 *                    you can think of 0 as equivalent to the 'x' axis, 1 being
 *                    equivalent to 'y', and so on...
 * @param index       array of index values (int) that refer to the specific
 *                    element, numbered according to the dimension. Using this
 *                    model, index 0 will always refer to the same actual
 *                    element (Place). However, an index of 1 will refer to the
 *                    first element along the dimension specified by 'dim'
 * @param indexSize   this is the physical number of elements that are being
 *                    referenced within the index[] array. This value is
 *                    necessary since there is no method to determine the size
 *                    of an array at runtime
 * @return            reference to (pointer) the location of the message queue,
 *                    which contains results from each call (size is equal to
 *                    ret_size argument - so, you can traverse using pointer
 *                    arithmetic)
 */
void *Places::callSome( int functionId, void *arguments[], int arg_size,
    int ret_size, int dim, int index[], int indexSize ) {
  if ( printOutput == true ) {
    cerr << "Places::callSome( int functionId, void *arguments[], int arg_size,"
        " int ret_size, int dim, int index[], int indexSize ) reached" << endl;
  }

  return cs_setup( functionId, (void *) arguments, arg_size, ret_size, dim,
      index, indexSize, Message::PLACES_CALL_SOME_VOID_OBJECT );
}

/**
 * This method helps reduce repeating code in the implementation of the various
 * callSome methods for Places.
 *
 * @param functionId  user-defined ID (handle) for corresponding function to
 *                    call in their user-defined Place class
 * @param argument    argument to pass into each Place method call
 * @param arg_size    total size of the argument
 * @param ret_size    size of each return value
 * @param dim         dimension of the array to call elements from. While
 *                    dimensions are typically considered in the form x, y, or
 *                    z, this value is expressed numerically (int). As such,
 *                    you can think of 0 as equivalent to the 'x' axis, 1 being
 *                    equivalent to 'y', and so on...
 * @param index       array of index values (int) that refer to the specific
 *                    element, numbered according to the dimension. Using this
 *                    model, index 0 will always refer to the same actual
 *                    element (Place). However, an index of 1 will refer to the
 *                    first element along the dimension specified by 'dim'.
 *                    Values are indexes into the single (flattened) index of
 *                    a particular Place. As such, the largest value will be
 *                    equal to the total number of Places in your simulation,
 *                    minus one (since zero-indexed into array)
 * @param indexSize   this is the physical number of elements that are being
 *                    referenced within the index[] array. This value is
 *                    necessary since there is no method to determine the size
 *                    of an array at runtime
 * @param type        the type of action to perform (see: Message::ACTION_TYPE)
 * @return            reference to (pointer) the location of the message queue,
 *                    which contains results from each call (size is equal to
 *                    ret_size argument - so, you can traverse using pointer
 *                    arithmetic)
 */
void *Places::cs_setup( int functionId, void *arguments, int arg_size,
    int ret_size, int dim, int index[], int indexSize,
    Message::ACTION_TYPE type ) {
  // calculate the total number of places we're calling for return object size
  int total = indexSize;    // assume total is equal to number in index[]
  bool rangeValues = false; // track whether a range of values is defined
  if ( indexSize == 3 ) {   // check to see if index[] refers to a range
    for ( int i = 0; i < indexSize && !rangeValues; i++ ) {
      if ( index[i] < 0 ) {
        total = size[i];    // element refers to a range in this dimension
        rangeValues = true; // flag that we're tracking a range (to parse later)
      }
    }
  }

  // calculate size of stripe - divide calls up between MNodes
  int stripe = total / MASS_base::systemSize; // systemSize: # of processes used

  // send PLACES_CALL_SOME_VOID_OBJECT message to each referenced place
  Message *m = NULL;
  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    // create a message
    if ( type == Message::PLACES_CALL_SOME_VOID_OBJECT )
      m = new Message( type, this->handle, functionId, arguments, arg_size,
          ret_size );
    else { // PLACES_CALL_SOME_RETURN_OBJECT
      m = new Message( type, this->handle, functionId,
      // argument body
          (char *) arguments + arg_size * stripe * ( i + 1 ),
          // argument size
          ( i == int( MASS::mNodes.size( ) ) - 1 ) ?
              arg_size * ( total - stripe * ( i + 1 ) ) : // + rmdr
              arg_size * stripe, ret_size ); // no remainder

      if ( printOutput == true ) {
        cerr << "Places.callSome: arg_size = " << arg_size << " stripe = "
            << stripe << " i + 1 = " << ( i + 1 ) << endl;
      }
    }

    // send it
    MASS::mNodes[i]->sendMessage( m );

    if ( printOutput == true ) {
      cerr << "PLACES_CALL_SOME " << m->getAction( ) << " sent to " << i
          << endl;
    }

    // make sure to delete it
    delete m;
  }

  // retrieve the corresponding places
  MASS_base::currentPlaces = this;
  MASS_base::currentFunctionId = functionId;
  MASS_base::currentArgument = arguments;
  MASS_base::currentArgSize = arg_size;
  MASS_base::currentMsgType = type;
  MASS_base::currentRetSize = ret_size;
  // prepare an entire return space
  MASS_base::currentReturns =
      ( type == Message::PLACES_CALL_SOME_VOID_OBJECT ) ?
          NULL : new char[total * MASS_base::currentRetSize];

  // resume threads
  Mthread::resumeThreads( Mthread::STATUS_CALLALL );

  // callSome implementation
  if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT )
    Places_base::callAll( functionId, arguments, 0 ); // 0 = the main thread id
  else
    Places_base::callAll( functionId, (void *) arguments, arg_size, ret_size,
        0 );

  // confirm all threads are done with callAll.
  Mthread::barrierThreads( 0 );

  // Synchronized with all slave processes
  if ( type == Message::PLACES_CALL_ALL_VOID_OBJECT )
    MASS::barrier_all_slaves( MASS_base::currentReturns, stripe,
        MASS_base::currentRetSize );
  else
    MASS::barrier_all_slaves( );

  return (void *) MASS_base::currentReturns;
}

/**
 * Calls from each of all cells to the method specified with functionId of
 * all destination cells, each indexed with a different Vector element.
 * Each vector element, say destination[] is an array of integers where
 * destination[i] includes a relative index (or a distance) on the coordinate
 * i from the current caller to the callee cell. The caller cell's outMessage
 * is a continuous set of arguments passed to the callee's method. The
 * caller's inMessages[] stores values returned from all callees. More
 * specifically, inMessages[i] maintains a set of return values from the i th
 * callee.
 *
 * @param dest_handle   a unique identifer that designates a group of Place
 *                      Objects as all belonging to the same simulation
 *                      (Places). Must be unique over all machines.
 * @param functionId    ID (int) of the function to call at each Place
 * @param destinations  a collection of relative offsets from each original
 *                      Place's location in the simulation. Using these
 *                      offsets, the simulation can call the actual Place
 *                      referenced (relative to the original Place) across the
 *                      entire simulation space - collecting the result(s) from
 *                      each of these neighbors in the original cell (Place)
 */
void Places::exchangeAll( int dest_handle, int functionId,
    vector<int*> *destinations ) {

  // send a PLACES_EXCHANGE_ALL message to each slave
  Message *m = new Message( Message::PLACES_EXCHANGE_ALL, this->handle,
      dest_handle, functionId, destinations, this->dimension );

  if ( printOutput == true ) {
    cerr << "dest_handle = " << dest_handle << endl;
  }

  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    MASS::mNodes[i]->sendMessage( m );
  }
  delete m;

  // retrieve the corresponding places
  MASS_base::currentPlaces = this;
  MASS_base::destinationPlaces = MASS_base::placesMap[dest_handle];
  MASS_base::currentFunctionId = functionId;
  MASS_base::currentDestinations = destinations;

  // reset requestCounter by the main thread
  MASS_base::requestCounter = 0;

  // for debug
  MASS_base::showHosts( );

  // resume threads
  Mthread::resumeThreads( Mthread::STATUS_EXCHANGEALL );

  // exchangeall implementation
  Places_base::exchangeAll( MASS_base::destinationPlaces, functionId,
      MASS_base::currentDestinations, 0 );

  // confirm all threads are done with exchangeAll.
  Mthread::barrierThreads( 0 );

  // Synchronized with all slave processes
  MASS::barrier_all_slaves( );
}

/**
 * This is similar to exchangeAll, but the method is confined to just exchange
 * existing data, the scope of affected Places is reduced, and the destinations
 * (offset) for each affected Place (boundary) is pre-defined to only trade
 * across this boundary to the corresponding "shadow cell" in the simulation
 * space.
 *
 * The shadow cell's outMessage is copied over to every boundary cell in the
 * simulation, enabling cross-boundary communication to occur.
 * 
 * @param dest_handle   a unique identifer that designates a group of Place
 *                      Objects as all belonging to the same simulation
 *                      (Places). Must be unique over all machines.
 * @param functionId    ID (int) of the function to call at each Place
 * @param destinations  a collection of relative offsets from each original
 *                      Place's location in the simulation. Using these
 *                      offsets, the simulation can call the actual Place
 *                      referenced (relative to the original Place) across the
 *                      entire simulation space - collecting the result(s) from
 *                      each of these neighbors in the original cell (Place)
 */
void Places::exchangeBoundary( ) {

  // send a PLACES_EXCHANGE_BOUNDARY message to each slave
  Message *m = new Message( Message::PLACES_EXCHANGE_BOUNDARY, this->handle,
      0 ); // 0 is dummy

  for ( int i = 0; i < int( MASS::mNodes.size( ) ); i++ ) {
    MASS::mNodes[i]->sendMessage( m );
  }
  delete m;

  // retrieve the corresponding places
  MASS_base::currentPlaces = this;

  // for debug
  MASS_base::showHosts( );

  // exchange boundary implementation
  Places_base::exchangeBoundary( );

  // Synchronized with all slave processes
  MASS::barrier_all_slaves( );
}
