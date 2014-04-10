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

void Agents_base::manageAll( int tid ) {
  
  //Create the dllclass to access our agents from, out agentsDllClass for agent
  //instantiation, and our bag for Agent objects after they have finished processing
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
    //Create the place with which to compare data to
    Place *evaluationPlace = evaluationAgent->place;

    //Iterate over all dimensions of the agent to check its location
    //against that of its place. If they are the same, returb back.
    int agentIndex = evaluationAgent->index.size();

    for( int i = 0; i < agentIndex ; i++){

      //If they match, keep iterating. If they all match, then loop will eventually end
      if(evaluationAgent->index[i] == evaluationPlace->index[i]){
	continue;
      }else{
	
	//If a number does not match, then we know we will need to migrate the agent	
	//Check if destination Place is within our current set of Places


	
	  int range[2];
	  Places_base *placesDllClass = MASS_base::placesMap[ placesHandle ];
	  placesDllClass->getLocalRange(range, tid);

	  /* for each neighbor
	      int *offset = (*destinations)[j];
	      int neighborCoord[dstPlaces->dimension];

	      // compute its coordinate
	      getGlobalNeighborArrayIndex( srcPlace->index, offset, dstPlaces->size,
					   dstPlaces->dimension, neighborCoord );

	      convert.str( "" );
	      convert << "tid[" << tid << "]: calls from"
		      << "[" << srcPlace->index[0]
		      << "][" << srcPlace->index[1] << "]"
		      << " (neighborCord[" << neighborCoord[0]
		      << "][" << neighborCoord[1] << "]"
		      << " dstPlaces->size[" << dstPlaces->size[0] 
		      << "][" << dstPlaces->size[1] << "]";

	      if ( neighborCoord[0] != -1 ) { 
		// destination valid
		int globalLinearIndex = 
		  getGlobalLinearIndexFromGlobalArrayIndex( neighborCoord,
							    dstPlaces->size,
							    dstPlaces->dimension );

		convert << " linear = " << globalLinearIndex
			<< " lower = " << dstPlaces->lower_boundary
			<< " upper = " << dstPlaces->upper_boundary << ")";

		if ( globalLinearIndex >= dstPlaces->lower_boundary &&
		     globalLinearIndex <= dstPlaces->upper_boundary ) {
		  // local destination
		  int destinationLocalLinearIndex 
		    = globalLinearIndex - dstPlaces->lower_boundary;
		  Place *dstPlace = 
		    (Place *)(dst_dllclass->places[destinationLocalLinearIndex]);

		  convert << " to [" << dstPlace->index[0]
			  << "][" << dstPlace->index[1] << "]";

		  // call the destination function
		  void *inMessage = dstPlace->callMethod( functionId, 
							  srcPlace->outMessage );

		  // store this inMessage: 
		  // note that callMethod must return a dynamic memory space
		  srcPlace->inMessages.push_back( inMessage );

		  // for debug
		  convert << " inMessage = " 
			  << *(int *)(srcPlace->inMessages.back( ));

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

		  convert.str( "" );
		  convert << "remoteRequest[" << destRank << "]->push_back:"
			  << " org = " << orgGlobalLinearIndex
			  << " dst = " << globalLinearIndex
			  << " size( ) = " 
			  << MASS_base::remoteRequests[destRank]->size( );
		  MASS_base::log( convert.str( ) );

		  pthread_mutex_unlock( &MASS_base::request_lock );
		}
	      } else {
		convert << " to destination invalid";
	      }

	      MASS_base::log( convert.str( ) );	
	    }
	  }
	}

	// all threads must barrier synchronize here.
	Mthread::barrierThreads( tid );
      */

      }
    }
  }

}




