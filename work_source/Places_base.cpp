/*
 MASS C++ Software License
 © 2014-2015 University of Washington
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
 © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "Places_base.h"
#include "MASS_base.h"
#include "Message.h"
#include "DllClass.h"
#include <iostream>
#include <sstream> // ostringstream

//Used to enable or disable output in places
const bool printOutput = false;
//const bool printOutput = true;


Places_base::Places_base( int handle, string className, void *argument,
		int argument_size, int dim, int size[] )
  : handle( handle ), className( className ), dimension( dim ) {

  if ( size == NULL ) 
    // if given in "int dim, ..." format, init_all() must be called later 
    return;
  this->size = new int[dim];
  for ( int i = 0; i < dim; i++ )
    this->size[i] = size[i];

  init_all( argument, argument_size );
}

Places_base::~Places_base( ) {
  // destroy( places ); to be debugged
  DllClass *dllclass = MASS_base::dllMap[handle];
  for ( int i = 0; i < places_size; i++ )
    dllclass->destroy( dllclass->places[i] );
  dlclose( dllclass->stub );
}

void Places_base::init_all( void *argument, int argument_size ) {
  // For debugging
  ostringstream convert;
  if(printOutput == true){
      convert << "handle = " << handle 
	      << ", class = " << className
	      << ", argument_size = " << argument_size 
	      << ", argument = " << (char *)argument
	      << ", dimension = " << dimension
	      << endl;
      for ( int i = 0; i < dimension; i++ )
        convert << "size[" << i << "] = " << size[i] << "  ";
      convert << endl;
      MASS_base::log( convert.str( ) );
  }

  // Print the current working directory
  /*
  char buf[200];
  getcwd( buf, 200 );
  convert.str( "" );
  convert << buf << endl;
  MASS_base::log( convert.str( ) );
  */

  // load the construtor and destructor
  DllClass *dllclass = new DllClass( className );
  MASS_base::dllMap.
    insert( map<int, DllClass*>::value_type( handle, dllclass ) );

  // calculate lower_boundary and upper_boundary
  int total = 1;
  for ( int i = 0; i < dimension; i++ )
    total *= size[i];
  int stripe = total / MASS_base::systemSize;

  lower_boundary = stripe * MASS_base::myPid;
  upper_boundary = ( MASS_base::myPid < MASS_base::systemSize - 1 ) ?
    lower_boundary + stripe - 1 : total - 1;
  places_size = upper_boundary - lower_boundary + 1;

  // instantiate Places objects within dlclass
  this->places_size = places_size;
  //  maintaining an entire set
  dllclass->places = new Place*[places_size]; 

  vector<int> index;
  index.reserve( dimension );

  for ( int i = 0; i < places_size; i++ ) {
    // instanitate a new place
    dllclass->places[i] = (Place *)( dllclass->instantiate( argument ) );  
    dllclass->places[i]->size.reserve( dimension );
    for ( int j = 0; j < dimension; j++ )
      // define size[] and index[]
      dllclass->places[i]->size.push_back( size[j] ); 
    dllclass->places[i]->index 
      = getGlobalArrayIndex( lower_boundary + i );
  }
}

/*! @brief Converts a given plain single index into a multi-dimensional index.

    @param[in] singleIndex an index in a plain single dimesion that will be
                           converted in a multi-dimensional index.
    @return a multi-dimensional index			   
 */
vector<int> Places_base::getGlobalArrayIndex( int singleIndex ) {
  vector<int> index;            // a multi-dimensional index
  index.reserve( dimension );
  vector<int>::iterator iter = index.begin();

  for ( int i = dimension - 1; i >= 0; i-- ) { 
    // calculate from lower dimensions
    index.insert( iter, 1, singleIndex % size[i] );
    singleIndex /= size[i];
  }

  return index;
}

void Places_base::callAll( int functionId, void *argument, int tid ) {
  int range[2];
  getLocalRange( range, tid );

  DllClass *dllclass = MASS_base::dllMap[ handle ];

  // debugging
  ostringstream convert;
  if ( printOutput == true ) {
    convert << "thread[" << tid << "] callAll functionId = " << functionId
	    << ", range[0] = " << range[0] << " range[1] = " << range[1]
	    << ", dllclass = " << (void *)dllclass;
    MASS_base::log( convert.str( ) );
  }

  if ( range[0] >= 0 && range[1] >= 0 ) {
    for ( int i = range[0]; i <= range[1]; i++ ) {
      if ( printOutput == true ) {
	convert.str( "" );
	convert << "thread[" << tid << "]: places[i] = " << dllclass->places[i];
	MASS_base::log( convert.str( ) );
      }
      dllclass->places[i]->callMethod( functionId, argument );
    }
  }
}

void **Places_base::callAll( int functionId, void *argument, 
			     int arg_size, int ret_size, int tid ) {
  int range[2];
  getLocalRange( range, tid );

  // debugging
  ostringstream convert;
  if ( printOutput == true ) {
    convert << "thread[" << tid << "] callAll_return object functionId = " 
	    << functionId
	    << ", range[0] = " << range[0] << " range[1] = " << range[1]
  	  << ", return_size = " << ret_size;
    MASS_base::log( convert.str( ) );
  }

  DllClass *dllclass = MASS_base::dllMap[ handle ];
  char *return_values = MASS_base::currentReturns + range[0] * ret_size;
  if ( range[0] >= 0 && range[1] >= 0 ) {
    for ( int i = range[0]; i <= range[1]; i++ ) {
      if ( printOutput == true ) {
	convert.str( "" );
	convert << "thread[" << tid << "]: places[i] = " << dllclass->places[i];
	MASS_base::log( convert.str( ) );
      }
      memcpy( (void *)return_values, dllclass->places[i]->callMethod( functionId, 
					      (char *)argument + arg_size * i
					      ), ret_size );
      return_values += ret_size;
    }
  }

  return NULL;
}

/*! @brief Returns the first and last of the range that should be allocated
           to a given thread

     @param[i] tid an id of the thread that calls this function.
     @return an array of two integers: element 0 = the first and 
             element 1 = the last
*/
void Places_base::getLocalRange( int range[], int tid ) {

  int nThreads = MASS_base::threads.size( );
  int portion = places_size / nThreads; // a range to be allocated per thread
  int remainder = places_size % nThreads;

  if ( portion == 0 ) {
    // there are more threads than elements in the MASS.Places_based object
    if ( remainder > tid ) {
      range[0] = tid;
      range[1] = tid;
    }
    else {
      range[0] = -1;
      range[1] = -1;
    }
  }
  else {
    // there are more MASS.Places than threads
    int first = tid * portion;
    int last = ( tid + 1 ) * portion - 1;
    if ( tid < remainder ) {
      // add in remainders
      first += tid;
      last = last + tid + 1; // 1 is one of remainders.
    }
    else {
      // remainders have been assigned to previous threads
      first += remainder;
      last += remainder;
    }
    range[0] = first;
    range[1] = last;
  }
}
// void Places_base::exchangeAll(Places_base *dstPlaces, int functionId, int tid){
//   exchangeAll( dstPlaces, functionId, , tid )
// }
void Places_base::exchangeAll( Places_base *dstPlaces, int functionId, 
			       vector<int*> *destinations, int tid ) {
  int range[2];
  getLocalRange( range, tid );
  ostringstream convert;
  // debugging
  if(printOutput == true){
      convert << "thread[" << tid << "] exchangeAll functionId = " << functionId
	      << ", range[0] = " << range[0] << " range[1] = " << range[1];
      MASS_base::log( convert.str( ) );
  }

  DllClass *src_dllclass = MASS_base::dllMap[ handle ];
  DllClass *dst_dllclass = MASS_base::dllMap[ dstPlaces->handle ];

  if(printOutput == true){
      convert.str( "" );
      convert << "tid[" << tid << "]: checks destinations:";
      for ( int i = 0; i < int( destinations->size( ) ); i++ ) {
        int *offset = (*destinations)[i];
        convert << "[" << offset[0]
	        << "][" << offset[1] << "]  ";    
      }
      MASS_base::log( convert.str( ) );
  }
  // now scan all places within range[0] ~ range[1]
  if ( range[0] >= 0 && range[1] >= 0 ) {
    for ( int i = range[0]; i <= range[1]; i++ ) {
      // for each place
      Place *srcPlace = (Place *)(src_dllclass->places[i]);

      // check its neighbors
      for ( int j = 0; j < int( destinations->size( ) ); j++ ) {

	// for each neighbor
	int *offset = (*destinations)[j];
	int neighborCoord[dstPlaces->dimension];

	// compute its coordinate
	getGlobalNeighborArrayIndex( srcPlace->index, offset, dstPlaces->size,
				     dstPlaces->dimension, neighborCoord );
	if(printOutput == true){
	    convert.str( "" );
	    convert << "tid[" << tid << "]: calls from"
		    << "[" << srcPlace->index[0]
		    << "][" << srcPlace->index[1] << "]"
		    << " (neighborCord[" << neighborCoord[0]
		    << "][" << neighborCoord[1] << "]"
		    << " dstPlaces->size[" << dstPlaces->size[0] 
		    << "][" << dstPlaces->size[1] << "]";
	    }
	if ( neighborCoord[0] != -1 ) { 
	  // destination valid
	  int globalLinearIndex = 
	    getGlobalLinearIndexFromGlobalArrayIndex( neighborCoord,
						      dstPlaces->size,
						      dstPlaces->dimension );
	  if(printOutput == true){
	      convert << " linear = " << globalLinearIndex
		      << " lower = " << dstPlaces->lower_boundary
		      << " upper = " << dstPlaces->upper_boundary << ")";
	  }

	  if ( globalLinearIndex >= dstPlaces->lower_boundary &&
	       globalLinearIndex <= dstPlaces->upper_boundary ) {
	    // local destination
	    int destinationLocalLinearIndex 
	      = globalLinearIndex - dstPlaces->lower_boundary;
	    Place *dstPlace = 
	      (Place *)(dst_dllclass->places[destinationLocalLinearIndex]);
	    
	    if(printOutput == true){
	        convert << " to [" << dstPlace->index[0]
		        << "][" << dstPlace->index[1] << "]";
	    }
	    // call the destination function
	    void *inMessage = dstPlace->callMethod( functionId, 
						    srcPlace->outMessage );

	    // store this inMessage: 
	    // note that callMethod must return a dynamic memory space
	    srcPlace->inMessages.push_back( inMessage );

	    // for debug
	    if(printOutput == true){
	        convert << " inMessage = " 
		        << *(int *)(srcPlace->inMessages.back( ));
	    }
	  } else {
	    // remote destination

	    // find the destination node
	    int destRank 
	      = getRankFromGlobalLinearIndex( globalLinearIndex );

	    // create a request
	    int orgGlobalLinearIndex =
	      getGlobalLinearIndexFromGlobalArrayIndex( &(srcPlace->index[0]), 
							size,
							dimension );
	    RemoteExchangeRequest *request 
	      = new RemoteExchangeRequest( globalLinearIndex,
					   orgGlobalLinearIndex,
					   j, // inMsgIndex
					   srcPlace->inMessage_size,
					   srcPlace->outMessage,
					   srcPlace->outMessage_size,
					   false );

	    // enqueue the request to this node.map
	    pthread_mutex_lock( &MASS_base::request_lock );
	    MASS_base::remoteRequests[destRank]->push_back( request );

	    if(printOutput == true){
	        convert.str( "" );
	        convert << "remoteRequest[" << destRank << "]->push_back:"
		        << " org = " << orgGlobalLinearIndex
		        << " dst = " << globalLinearIndex
		        << " size( ) = " 
		        << MASS_base::remoteRequests[destRank]->size( );
	        MASS_base::log( convert.str( ) );
	    }
	    pthread_mutex_unlock( &MASS_base::request_lock );
	  }
	} else {
	  if(printOutput == true)
	    convert << " to destination invalid";
	}
	if(printOutput == true){
	    MASS_base::log( convert.str( ) );
	}	
      }
    }
  }

  // all threads must barrier synchronize here.
  Mthread::barrierThreads( tid );
  if ( tid == 0 ) {

    if(printOutput == true){
        convert.str( "" );
        convert << "tid[" << tid << "] now enters processRemoteExchangeRequest";
        MASS_base::log( convert.str( ) );
    }
    // the main thread spawns as many communication threads as the number of
    // remote computing nodes and let each invoke processRemoteExchangeReq.
    
    // args to threads: rank, srcHandle, dstHandle, functionId, lower_boundary
    int comThrArgs[MASS_base::systemSize][5];
    pthread_t thread_ref[MASS_base::systemSize]; // communication thread id
    for ( int rank = 0; rank < MASS_base::systemSize; rank++ ) {

      if ( rank == MASS_base::myPid ) // don't communicate with myself
	continue;

      // set arguments 
      comThrArgs[rank][0] = rank;
      comThrArgs[rank][1] = handle;
      comThrArgs[rank][2] = dstPlaces->handle;
      comThrArgs[rank][3] = functionId;
      comThrArgs[rank][4] = lower_boundary;

      // start a communication thread
      if ( pthread_create( &thread_ref[rank], NULL, 
			   Places_base::processRemoteExchangeRequest, 
			   comThrArgs[rank] ) != 0 ) {
	MASS_base::log( "Places_base.exchangeAll: failed in pthread_create" );
	exit( -1 );
      }
    }

    // wait for all the communication threads to be terminated
    for ( int rank = 0; rank < MASS_base::systemSize; rank++ ) {
      if ( rank == MASS_base::myPid ) // don't communicate with myself
	continue;      
      pthread_join( thread_ref[rank], NULL );
    }
  }
  else {
    if(printOutput == true){
        convert.str( "" );
        convert << "tid[" << tid << "] skips processRemoteExchangeRequest";
        MASS_base::log( convert.str( ) );
    }
  }

}

void *Places_base::processRemoteExchangeRequest( void *param ) {
  int destRank = ( (int *)param )[0];
  int srcHandle = ( (int *)param )[1];
  int destHandle_at_src = ( (int *)param )[2];
  int functionId = ( (int *)param )[3];
  int my_lower_boundary = ( (int *)param )[4];

  vector<RemoteExchangeRequest*>* orgRequest = NULL;
  ostringstream convert;

  if(printOutput == true){
      convert.str( "" );
      convert << "rank[" << destRank << "]: starts processRemoteExchangeRequest";
      MASS_base::log( convert.str( ) );
  }
  // pick up the next rank to process
  orgRequest = MASS_base::remoteRequests[destRank];

  // for debugging
  pthread_mutex_lock( &MASS_base::request_lock );

  if(printOutput == true){
      convert.str( "" );
      convert << "tid[" << destRank << "] sends an exhange request to rank: " 
	      << destRank << " size() = " << orgRequest->size( ) << endl;
      for ( int i = 0; i < int( orgRequest->size( ) ); i++ ) {
        convert << "send from "
	        << (*orgRequest)[i]->orgGlobalLinearIndex << " to "
	        << (*orgRequest)[i]->destGlobalLinearIndex << " at "
	        << (*orgRequest)[i]->inMessageIndex << " inMsgSize: " 
	        << (*orgRequest)[i]->inMessageSize << " outMsgSize: " 
	        << (*orgRequest)[i]->outMessageSize << endl;
      }
      MASS_base::log( convert.str( ) );
  }
  pthread_mutex_unlock( &MASS_base::request_lock );

  // now compose and send a message by a child
  Message *messageToDest 
    = new Message( Message::PLACES_EXCHANGE_ALL_REMOTE_REQUEST,
		   srcHandle, destHandle_at_src, functionId, orgRequest );

  struct ExchangeSendMessage rankNmessage;
  rankNmessage.rank = destRank;
  rankNmessage.message = messageToDest;
  pthread_t thread_ref;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );

  // receive a message by myself
  Message *messageFromSrc = MASS_base::exchange.receiveMessage( destRank );

  // at this point, 
  // at this point, the message must be exchanged.
  pthread_join( thread_ref, NULL );

  // process a message
  vector<RemoteExchangeRequest*>* receivedRequest 
    = messageFromSrc->getExchangeReqList( );

  int destHandle_at_dst = messageFromSrc->getDestHandle( );
  Places_base *dstPlaces = MASS_base::placesMap[ destHandle_at_dst ];
  DllClass *dst_dllclass = MASS_base::dllMap[ destHandle_at_dst ];

  if(printOutput == true){
      convert.str( "" );
      convert << "request from rank[" << destRank << "] = " << receivedRequest;
      convert << " size( ) = " << receivedRequest->size( );
  }

  // get prepared for a space to sotre return values
  int inMessageSizes = 0;
  for ( int i = 0; i < int( receivedRequest->size( ) ); i++ ) 
    inMessageSizes += (*receivedRequest)[i]->inMessageSize;
  char retVals[inMessageSizes];
  
  if(printOutput == true){
      convert << " retVals = " << (void *)retVals
	      << " total inMessageSizes = " << inMessageSizes << endl;
  }
  // for each place, call the corresponding callMethod( ).
  char *retValPos = retVals;
  for ( int i = 0; i < int( receivedRequest->size( ) ); i++ ) {

    if(printOutput == true){
        convert << "received from "
	        << (*receivedRequest)[i]->orgGlobalLinearIndex << " to "
	        << (*receivedRequest)[i]->destGlobalLinearIndex << " at "
	        << (*receivedRequest)[i]->inMessageIndex << " inMsgSize: " 
	        << (*receivedRequest)[i]->inMessageSize << " outMsgSize: "
	        << (*receivedRequest)[i]->outMessageSize 
	        << " dstPlaces->lower = " << dstPlaces->lower_boundary
	        << " dstPlaces->upper = " << dstPlaces->upper_boundary;
    }
    int globalLinearIndex = (*receivedRequest)[i]->destGlobalLinearIndex;
    void *outMessage = (*receivedRequest)[i]->outMessage;
    
    if ( globalLinearIndex >= dstPlaces->lower_boundary &&
	 globalLinearIndex <= dstPlaces->upper_boundary ) {
      // local destination
      int destinationLocalLinearIndex 
	= globalLinearIndex - dstPlaces->lower_boundary;
      
      if(printOutput == true)
          convert << " dstLocal = " << destinationLocalLinearIndex << endl;
      
      Place *dstPlace = 
	(Place *)(dst_dllclass->places[destinationLocalLinearIndex]);
      
      // call the destination function
      void *inMessage = dstPlace->callMethod( functionId, 
					      outMessage );
      memcpy(  retValPos, inMessage, (*receivedRequest)[i]->inMessageSize );
      retValPos += (*receivedRequest)[i]->inMessageSize;
    }
  }
  if(printOutput == true){
      MASS_base::log( convert.str( ) );
  }
  delete messageFromSrc;

  // send return values by a child thread
  Message *messageToSrc = 
    new Message( Message::PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT,
		 retVals, inMessageSizes );
  rankNmessage.rank = destRank;
  rankNmessage.message = messageToSrc;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );
  
  // receive return values by myself in parallel
  Message *messageFromDest = MASS_base::exchange.receiveMessage( destRank );

  // at this point, the message must be exchanged.
  pthread_join( thread_ref, NULL );
  delete messageToSrc;

  // store return values to the orignal places
  int argumentSize = messageFromDest->getArgumentSize( );
  char argument[argumentSize];
  messageFromDest->getArgument( argument );

  int pos = 0;
  DllClass *src_dllclass = MASS_base::dllMap[ srcHandle ];
  
  if(printOutput == true){
      convert.str( "" );
      convert << "return values: argumentSize = " << argumentSize
	      << " src_dllclass = " << src_dllclass
	      << " orgRequest->size( ) = " << orgRequest->size( );
      MASS_base::log( convert.str( ) );
  }
  
  for ( int i = 0; i < int( orgRequest->size( ) ); i++ ) {
    // local source
    int orgLocalLinearIndex
      = (*orgRequest)[i]->orgGlobalLinearIndex - my_lower_boundary;
    
    // locate a local place
    Place *srcPlace =	(Place *)(src_dllclass->places[orgLocalLinearIndex]);
    
    // store a return value to it
    char *inMessage = new char[srcPlace->inMessage_size];
    memcpy( inMessage, argument + pos, srcPlace->inMessage_size );
    pos += srcPlace->inMessage_size;
    
    // insert an item at inMessageIndex or just append it.
    if ( int( srcPlace->inMessages.size( ) ) 
	 > (*orgRequest)[i]->inMessageIndex )
      srcPlace->inMessages.insert( srcPlace->inMessages.begin( ) 
				   + (*orgRequest)[i]->inMessageIndex,
				   (void *)inMessage );
    else
      srcPlace->inMessages.push_back( (void *)inMessage );

    if ( printOutput == true ) {
      convert.str( "" );
      convert << "srcPlace[" << srcPlace->index[0]<< "][" 
	      << srcPlace->index[1] << "] inserted " 
	      << "at " << (*orgRequest)[i]->inMessageIndex;
      MASS_base::log( convert.str( ) );
    }
    
  }
  delete messageToDest; // messageToDest->orgReuqest is not more used. delete it.
  delete messageFromDest;

  return NULL;
}

void *Places_base::sendMessageByChild( void *param ) {
  int rank = ((struct ExchangeSendMessage *)param)->rank;
  Message *message = (Message *)((struct ExchangeSendMessage *)param)->message;
  MASS_base::exchange.sendMessage( rank, message );
  return NULL;
}

void Places_base::getGlobalNeighborArrayIndex( vector<int>src_index, 
					       int offset[],
					       int dst_size[], 
					       int dest_dimension,
					       int dest_index[] ) {
  for (int i = 0; i < dest_dimension; i++ ) {
    dest_index[i] = src_index[i] + offset[i]; // calculate dest index

    if ( dest_index[i] < 0 || dest_index[i] >= dst_size[i] ) {
      // out of range
      for ( int j = 0; j < dest_dimension; j++ ) {
	// all index must be set -1
	dest_index[j] = -1;
	return;
      }
    }
  }
}

int Places_base::getGlobalLinearIndexFromGlobalArrayIndex( int index[], 
							   int size[],
							   int dimension ) {
  int retVal = 0;

  for ( int i = 0; i < dimension; i++ ) {
    if ( index[i] >= 0 && size[i] > 0 && index[i] < size[i] ) {
      retVal = retVal * size[i];
      retVal += index[i];
    }
  }

  return retVal;
}

int Places_base::getRankFromGlobalLinearIndex( int globalLinearIndex ) {
  static int total = 0;
  static int stripe = 0;

  if ( total == 0 ) {
    // first time computation
    total = 1;
    for ( int i = 0; i < dimension; i++ )
      total *= size[i];
    stripe = total / MASS_base::systemSize;
  }

  int rank, scope;
  for ( rank = 0, scope = stripe ; rank < MASS_base::systemSize; 
	rank++, scope += stripe ) {
    if ( globalLinearIndex < scope )
      break;
  }

  return ( rank == MASS_base::systemSize ) ? rank - 1 : rank;
}
