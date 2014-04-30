#include "Agents_base.h"
#include "MASS_base.h"
#include "DllClass.h"
#include <sstream> // ostringstream
#include <vector>

Agents_base::Agents_base( int handle, string className, void *argument,
			  int argument_size, int placesHandle, 
			  int initPopulation ) 
  : handle( handle ), className( className ), 
    placesHandle( placesHandle ), initPopulation( initPopulation ) {

  // For debugging
  ostringstream convert;

  convert << "handle = " << handle
	  << ",placesHandle = " << placesHandle
          << ", class = " << className
          << ", argument_size = " << argument_size
          << ", argument = " << (char *)argument
          << ", initPopulation = " << initPopulation
          << endl;
  MASS_base::log( convert.str( ) );

  // load the construtor and destructor
  DllClass *agentsDllClass = new DllClass( className );
  MASS_base::dllMap.
    insert( map<int, DllClass*>::value_type( handle, agentsDllClass ) );

  // initialize currentAgentId and localPopulation
  currentAgentId = MASS_base::myPid * MAX_AGENTS_PER_NODE;
  localPopulation = 0;

  // instantiate just one agent to call its map( ) function
  Agent *protoAgent = (Agent *)( agentsDllClass->instantiate( argument ) );

  // retrieve the corresponding places
  DllClass *placesDllClass = MASS_base::dllMap[ placesHandle ];
  Places_base *curPlaces = MASS_base::placesMap[ placesHandle ];

  convert.str( "" );
  convert << "Agets_base constructor: placesDillClass = " 
	  << (void *)placesDllClass
	  << " curPlaces = " << (void *)curPlaces;
  MASS_base::log( convert.str( ) );

  for ( int i = 0; i < curPlaces->getPlacesSize( ); i++ ) {

    // scan each place to see how many agents it can create
    Place *curPlace = placesDllClass->places[i];

    convert.str( "" );
    convert << "Agent_base constructor place[" << i << "]";
    MASS_base::log( convert.str( ) );

    // create as many new agents as nColonists
    for ( int nColonists = 
	    protoAgent->map( initPopulation, curPlace->size, curPlace->index );
	  nColonists > 0; nColonists--, localPopulation++ ) {
      
      // agent instanstantiation and initialization
      Agent *newAgent = (Agent *)(agentsDllClass->instantiate( argument ) );

      convert.str( "" );
      convert << " newAgent[" << localPopulation << "] = " << (void *)newAgent;
      MASS_base::log( convert.str( ) );

      newAgent->agentsHandle = handle;
      newAgent->placesHandle = placesHandle;
      newAgent->agentId = currentAgentId++;
      newAgent->parentId = -1; // no parent
      newAgent->place = curPlace;

      for ( int index = 0; index < int( curPlace->index.size( ) ); index++ )
	newAgent->index.push_back( curPlace->index[index] );
      newAgent->alive = true;

      // store this agent in the bag of agents
      agentsDllClass->agents->push_back( newAgent );

      // TODO: register newAgent into curPlace
      curPlace->agents.push_back( (MObject *)newAgent );
    }
  }
  delete protoAgent;
}

Agents_base::~Agents_base( ) {
}

void Agents_base::callAll( int functionId, void *argument, int tid ) {

	//Chris ToDo: Implement
	//Set up the bag of agents
	DllClass *dllclass = MASS_base::dllMap[ handle ];

	//Create the bag for returning agents to be placed in
	vector<Agent*> retBag;
	int numOfOriginalVectors = Mthread::agentBagSize;
	ostringstream convert;

	while (true){
		//Create the index for this iteration
		int myIndex;

		//Lock the index assignment so no two threads will receive the same value
		pthread_mutex_lock(&MASS_base::request_lock);

		/**/		//Thread checking
		convert.str("");
		convert << "Starting index value is: " << Mthread::agentBagSize;
		MASS_base::log(convert.str());
		/**/

		myIndex=Mthread::agentBagSize--;


		//Error Checking
		convert.str("");
		convert << "Thread[" << myIndex << "] assigned";
		MASS_base::log(convert.str());

		pthread_mutex_unlock(&MASS_base::request_lock);

		//Continue to run until the assigning index becomes negative
		//(in which case, we've run out of agents)
		if(myIndex > 0){
			convert.str("");			
			//Lock the assignment and removal of agents
			pthread_mutex_lock(&MASS_base::request_lock);
			convert << "Thread [" << myIndex << "] is being removed; ";
			Agent* tmpAgent = dllclass->agents->back();
			dllclass->agents->pop_back();
			pthread_mutex_unlock(&MASS_base::request_lock);
			
			//Use the Agents' callMethod to have it begin running
			tmpAgent->callMethod(functionId, argument);
			convert << "Thread [" << myIndex << "] has called its method; ";
			
			//Puth the now running thread into the return bag
			retBag.push_back(tmpAgent);
			convert << "Thread [" << myIndex << "] has been placed in the return bag; ";
			convert << "Current Agent Bag Size is: " << Mthread::agentBagSize;
			MASS_base::log(convert.str());
		}
		//Otherwise, we are out of agents and should stop
		//trying to assign any more
		else{
			break;
		}
	}
	//Wait for the thread count to become zero
	Mthread::barrierThreads( 0 );
	
	//Assign the new bag of finished agents to the old pointer for reuse
	MASS_base::dllMap[ handle ]->agents = &retBag;
	Mthread::agentBagSize = numOfOriginalVectors;

}

void **Agents_base::callAll( int functionId, void *argument, int arg_size,
			     int ret_size, int tid ) {

	  // Chris ToDo: Replace old bag with new bag when threads terminate.
	  // Chris ToDo: Check return values for accuracy
	  // Chris ToDo: Confirm barrier threads work as intended
	  // Chris Todo: make sure new vector is compatable with old one

	  ostringstream convert;
	  vector<Agent*> retBag;

	  DllClass *dllclass = MASS_base::dllMap[ handle ];
	  char *return_values = MASS_base::currentReturns + (Mthread::agentBagSize - 1) * ret_size;
	  while(true){
		  int myIndex;
		  pthread_mutex_lock(&MASS_base::request_lock);
		  myIndex=Mthread::agentBagSize--;
		  pthread_mutex_unlock(&MASS_base::request_lock);			

		  //While there are still indexes left, continue to grab and execute threads
		  //When all are executing, place into vector and wait for them to finish
		  if(myIndex >= 0){
			  pthread_mutex_lock(&MASS_base::request_lock);
			  Agent* tmpAgent = dllclass->agents->back();
			  dllclass->agents->pop_back();
			  pthread_mutex_unlock(&MASS_base::request_lock);
			  convert << "Thread [" << myIndex << "] has been removed";


			  tmpAgent->callMethod(functionId, (void *)return_values);
			  convert << "Thread [" << myIndex << "] has been called";

			  
			  retBag.push_back(tmpAgent);
			  convert << "Thread [" <<  myIndex << "] has been moved to new bag";

		  }else{
			  break;
		  }
	  }
	  //Confirm all threads have finished
	  Mthread::barrierThreads( 0 );

	  //Reassign the bag after execution to be reused later
	  MASS_base::dllMap[ handle ]->agents = &retBag;
	  return NULL;
}

void Agents_base::getGlobalAgentArrayIndex( vector<int> src_index,
				    int dst_size[], int dest_dimension,
				    int dest_index[] ){

 for (int i = 0; i < dest_dimension; i++ ) {
    dest_index[i] = src_index[i]; // calculate dest index

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

void Agents_base::manageAll( int tid ) {
  
  //Create the dllclass to access our agents from, out agentsDllClass for agent
  //instantiation, and our bag for Agent objects after they have finished processing
  ostringstream convert;
  DllClass *dllclass = MASS_base::dllMap[handle];
  DllClass *agentsDllClass = new DllClass( className );
  vector<Agent*> retBag;

  //Chris ToDo: Spawn, then Kill, then Migrate. Check in that order throughout the 
  //bag of agents sequentially.
  while( true ){

    pthread_mutex_lock(&MASS_base::request_lock);
    int agentSize;
    agentSize = dllclass->agents->size();
    pthread_mutex_unlock(&MASS_base::request_lock);

    if(agentSize ==0){
      break;
    }
    //Grab the last agent and remove it for processing. Be sure to lock on removal
    pthread_mutex_lock(&MASS_base::request_lock);
    Agent *evaluationAgent = dllclass->agents->back();
    dllclass->agents->pop_back();
    pthread_mutex_unlock(&MASS_base::request_lock);
    int argumentcounter = 0;

    //If the spawn's newChildren field is set to anything higher than zero
    //we need to create newChildren's worth of Agents in the current location.

    //Spawn() Check
    //    pthread_mutex_lock(&MASS_base::request_lock);
    int childrenCounter = evaluationAgent->newChildren;
    //   pthread_mutex_unlock(&MASS_base::request_lock);

    while( childrenCounter > 0 ){
      Agent* addAgent = (Agent *)(agentsDllClass->instantiate(evaluationAgent->arguments[argumentcounter++]));
      
      //Push the created agent into our bag for returns and update the counter
      //needed to keep track of our agents.

      //Lock here
       pthread_mutex_lock(&MASS_base::request_lock);
       retBag.push_back(addAgent);
       Mthread::agentBagSize += 1;
       pthread_mutex_unlock(&MASS_base::request_lock);

       //Push the pointer copy into the current Agent's place location
       evaluationAgent->place->agents.push_back((MObject*) addAgent);

       //Copy the parent index to the child
       addAgent->index = evaluationAgent->index;

       //Set the new Agent's current Place to the parent's Place
       addAgent->place = evaluationAgent->place;

       //Copy the current Agent's parent to the new Agent
       addAgent->parentId = evaluationAgent->agentId;

      //Decrement the newChildren counter once an Agent has been spawned
      evaluationAgent->newChildren -= 1;
      childrenCounter -= 1;
    }

    //Kill() Check
    if(evaluationAgent->alive == false){
      
      //Get the place in which evaluationAgent is 'stored' in
      Place *evaluationPlace = evaluationAgent->place;
      
      //Move through the list of Agents to locate which to delete
      pthread_mutex_lock(&MASS_base::request_lock);
      int evalPlaceAgents = evaluationPlace->agents.size();
      pthread_mutex_unlock(&MASS_base::request_lock);

      for( int i = 0; i < evalPlaceAgents; i++){
	
	//Type casting used so we can compare agentId's
	MObject *comparisonAgent = evaluationPlace->agents[i];
	Agent *convertedAgent = static_cast<Agent*>(comparisonAgent);

	//Check the Id against the ID of the agent to be removed. If it matches, remove it
	//Lock
	if((evaluationAgent->agentId == convertedAgent->agentId) && (evaluationAgent->agentsHandle == convertedAgent->agentsHandle)){
	  pthread_mutex_lock(&MASS_base::request_lock);
	  evaluationPlace->agents.erase( evaluationPlace->agents.begin() + i );
	  pthread_mutex_unlock(&MASS_base::request_lock);
	  break;
	}
      }
      //Delete the agent and its pointer to complete the removal
      delete &evaluationAgent;
      delete evaluationAgent;
      return;
    }

    //Migrate() check

    //Iterate over all dimensions of the agent to check its location
    //against that of its place. If they are the same, returb back.
    int agentIndex = evaluationAgent->index.size();

    Places_base *placesDllClass = MASS_base::placesMap[ placesHandle ];
    int destCoord[agentIndex];

	// compute its coordinate
    getGlobalAgentArrayIndex( evaluationAgent->index, placesDllClass->size,
			      placesDllClass->dimension, destCoord );

    convert.str( "" );
    convert << "tid[" << tid << "]: calls from"
	    << "[" << evaluationAgent->index[0]
	    << "][" << evaluationAgent->index[1] << "]"
	    << " (neighborCord[" << destCoord[0]
	    << "][" << destCoord[1] << "]"
	    << " placesDllClass->size[" << placesDllClass->size[0] 
	    << "][" << placesDllClass->size[1] << "]";

    if( destCoord[0] != -1 ) { 
      // destination valid
      int globalLinearIndex = 
	placesDllClass->getGlobalLinearIndexFromGlobalArrayIndex( destCoord,
						  placesDllClass->size,
						  placesDllClass->dimension );

      convert << " linear = " << globalLinearIndex
	      << " lower = " << placesDllClass->lower_boundary
	      << " upper = " << placesDllClass->upper_boundary << ")";

      if ( globalLinearIndex >= placesDllClass->lower_boundary &&
	   globalLinearIndex <= placesDllClass->upper_boundary ) {
	// local destination
	int destinationLocalLinearIndex 
	  = globalLinearIndex - placesDllClass->lower_boundary;

	evaluationAgent->place = dllclass->places[destinationLocalLinearIndex];
	pthread_mutex_lock(&MASS_base::request_lock);
	evaluationAgent->place->agents.push_back((MObject *)evaluationAgent);
	pthread_mutex_unlock(&MASS_base::request_lock);

	// CHRIS, WHAT DO THESE THREE STATEMENTS DO? 4-29-14
	// *Should remove the pointer object in the place that points to the migrting Agent

	// pthread_mutex_lock(&MASS_base::request_lock);
	// evaluationPlace->agents.erase( evaluationPlace->agents.begin() + i );
	// pthread_mutex_unlock(&MASS_base::request_lock);

	// for debug
	//	    convert << " inMessage = " 
	//		    << *(int *)(evaluationPlace->inMessages.back( ));

      } 
      else {
	// remote destination

	// find the destination node
	int destRank 
	  = placesDllClass->getRankFromGlobalLinearIndex( globalLinearIndex );

	// create a request
	AgentMigrationRequest *request 
	  = new AgentMigrationRequest( globalLinearIndex, evaluationAgent );
	
	// enqueue the request to this node.map
	pthread_mutex_lock( &MASS_base::request_lock );
	MASS_base::migrationRequests[destRank]->push_back( request );
	
	convert.str( "" );
	convert << "remoteRequest[" << destRank << "]->push_back:"
		<< " dst = " << globalLinearIndex;
	MASS_base::log( convert.str( ) );
	
	pthread_mutex_unlock( &MASS_base::request_lock );
      }
    } else {
      convert << " to destination invalid";
    }
    MASS_base::log( convert.str( ) );	
    
    // all threads must barrier synchronize here.
    Mthread::barrierThreads( tid );
    if ( tid == 0 ) {
      
      convert.str( "" );
      convert << "tid[" << tid << "] now enters processAgentMigrationRequest";
      MASS_base::log( convert.str( ) );
      
      // the main thread spawns as many communication threads as the number of
      // remote computing nodes and let each invoke processAgentMigrationReq.
      
      // args to threads: rank, agentHandle, placeHandle, lower_boundary
      int comThrArgs[MASS_base::systemSize][4];
      pthread_t thread_ref[MASS_base::systemSize]; // communication thread id
      for ( int rank = 0; rank < MASS_base::systemSize; rank++ ) {
	
	if ( rank == MASS_base::myPid ) // don't communicate with myself
	  continue;
	
	// set arguments 
	comThrArgs[rank][0] = rank;
	comThrArgs[rank][1] = handle; // agents' handle
	comThrArgs[rank][2] = placesDllClass->handle;
	comThrArgs[rank][3] = placesDllClass->lower_boundary;
	
	// start a communication thread
	if ( pthread_create( &thread_ref[rank], NULL, 
			     Agents_base::processAgentMigrationRequest, 
			     comThrArgs[rank] ) != 0 ) {
	  MASS_base::log( "Agents_base.manageAll: failed in pthread_create" );
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
      
      convert.str( "" );
      convert << "tid[" << tid << "] skips processAgentMigrationRequest";
      MASS_base::log( convert.str( ) );

    return;
  } // end of while( true ) 
}

  // CHRIS, WHAT DO THESE THREE STATEMENTS DO? 4-29-14
  //If not killed or migrated, push Agent into the retBag  
  // pthread_mutex_lock(&MASS_base::request_lock);
  // retBag.push_back(evaluationAgent);
  // pthread_mutex_unlock(&MASS_base::request_lock);

  //Need to hook the bag back up when done
}

//Chris: Additional Code below:

void *Agents_base::processAgentMigrationRequest( void *param ) {
  int destRank = ( (int *)param )[0];
  int agentHandle = ( (int *)param )[1];
  int placeHandle = ( (int *)param )[2];
  //  int my_lower_boundary = ( (int *)param )[3];

  vector<AgentMigrationRequest*>* orgRequest = NULL;
  ostringstream convert;

  convert.str( "" );
  convert << "rank[" << destRank << "]: starts processAgentMigrationRequest";
  MASS_base::log( convert.str( ) );

  // pick up the next rank to process
  orgRequest = MASS_base::migrationRequests[destRank];

  // for debugging
  pthread_mutex_lock( &MASS_base::request_lock );
  convert.str( "" );
  convert << "tid[" << destRank << "] sends an exhange request to rank: " 
	  << destRank << " size() = " << orgRequest->size( ) << endl;
  for ( int i = 0; i < int( orgRequest->size( ) ); i++ ) {
    convert << "send "
	    << (*orgRequest)[i]->agent << " to "
	    << (*orgRequest)[i]->destGlobalLinearIndex << endl;
  }
  MASS_base::log( convert.str( ) );
  pthread_mutex_unlock( &MASS_base::request_lock );

  // now compose and send a message by a child
  Message *messageToDest 
    = new Message( Message::AGENTS_MIGRATION_REMOTE_REQUEST,
		   agentHandle, placeHandle, orgRequest );

  struct MigrationSendMessage rankNmessage;
  rankNmessage.rank = destRank;
  rankNmessage.message = messageToDest;
  pthread_t thread_ref;
  pthread_create( &thread_ref, NULL, sendMessageByChild, &rankNmessage );

  // receive a message by myself
  Message *messageFromSrc = MASS_base::exchange.receiveMessage( destRank );
 
  // at this point, the message must be exchanged.
  pthread_join( thread_ref, NULL );
  delete messageToDest;

  ///////////////////////////////////////////////////////////////////////
  // process a message
  vector<AgentMigrationRequest*>* receivedRequest 
    = messageFromSrc->getMigrationReqList( );

  int agentsHandle = messageFromSrc->getHandle( );
  //  Agents_base *dstAgents = MASS_base::agentsMap[ agentsHandle ];
  int placesHandle = messageFromSrc->getDestHandle( );
  Places_base *dstPlaces = MASS_base::placesMap[ placesHandle ];
  DllClass *agents_dllclass = MASS_base::dllMap[ agentsHandle ];
  DllClass *places_dllclass = MASS_base::dllMap[ placesHandle ];

  convert.str( "" );
  convert << "request from rank[" << destRank << "] = " << receivedRequest;
  convert << " size( ) = " << receivedRequest->size( );

  // retrieve agents from receiveRequest
  while( receivedRequest->size( ) > 0 ) {
    AgentMigrationRequest *request = receivedRequest->back( );
    receivedRequest->pop_back( );
    int globalLinearIndex = request->destGlobalLinearIndex;
    Agent *agent = request->agent;

    // local destination
    int destinationLocalLinearIndex 
      = globalLinearIndex - dstPlaces->lower_boundary;
      
    convert << " dstLocal = " << destinationLocalLinearIndex << endl;
    
    Place *dstPlace = 
      (Place *)(places_dllclass->places[destinationLocalLinearIndex]);

    // push this agent into the place and the entire agent bag.
    agent->index = dstPlace->index;
    dstPlace->agents.push_back( agent );
    agents_dllclass->agents->push_back( agent );

    delete request;
  }

  MASS_base::log( convert.str( ) );
  delete messageFromSrc;

  return NULL;
}

void *Agents_base::sendMessageByChild( void *param ) {
  int rank = ((struct MigrationSendMessage *)param)->rank;
  Message *message = (Message *)((struct MigrationSendMessage *)param)->message;
  MASS_base::exchange.sendMessage( rank, message );
  return NULL;
}






