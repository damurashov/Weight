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

#include "Agents_base.h"
#include "MASS_base.h"
#include "DllClass.h"
#include <sstream> // ostringstream
#include <vector>
using namespace std;

//Used to enable or disable output for Agents
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

/**
 *
 * @param handle
 * @param className
 * @param argument
 * @param argument_size
 * @param placesHandle
 * @param initPopulation
 **/
Agents_base::Agents_base(int handle, string className, void *argument,
	int argument_size, int placesHandle,
	int initPopulation)
	: handle(handle), className(className),
	placesHandle(placesHandle), initPopulation(initPopulation) {

	// For debugging
	ostringstream convert;
	if (printOutput == true) {
		convert << "handle = " << handle
			<< ",placesHandle = " << placesHandle
			<< ", class = " << className
			<< ", argument_size = " << argument_size
			<< ", argument = " << (char *)argument
			<< ", initPopulation = " << initPopulation
			<< endl;
		MASS_base::log(convert.str());
	}

	// load the construtor and destructor
	DllClass *agentsDllClass = new DllClass(className);
	MASS_base::dllMap.
		insert(map<int, DllClass *>::value_type(handle, agentsDllClass));

	// initialize currentAgentId and localPopulation
	currentAgentId = MASS_base::myPid * MAX_AGENTS_PER_NODE;
	localPopulation = 0;

	// instantiate just one agent to call its map( ) function
	Agent *protoAgent = (Agent *)(agentsDllClass->instantiate(argument));

	// retrieve the corresponding places
	DllClass *placesDllClass = MASS_base::dllMap[placesHandle];
	Places_base *curPlaces = MASS_base::placesMap[placesHandle];

	if (printOutput == true) {
		convert.str("");
		convert << "Agets_base constructor: placesDllClass = "
			<< (void *)placesDllClass
			<< " curPlaces = " << (void *)curPlaces;
		MASS_base::log(convert.str());
	}
	for (int i = 0; i < curPlaces->getPlacesSize(); i++) {

		// scan each place to see how many agents it can create
		Place *curPlace = placesDllClass->places[i];

		if (printOutput == true) {
			convert.str("");
			convert << "Agent_base constructor place[" << i << "]";
			convert << "dllClass->places[" << i << "] = " << curPlace;
			MASS_base::log(convert.str());
		}
		// create as many new agents as nColonists
		for (int nColonists =
			protoAgent->map(initPopulation, curPlace->size, curPlace->index, curPlace);
			nColonists > 0; nColonists--, localPopulation++) {
			if (printOutput) {
				convert.str("");
				convert << "Finished call to map on protoAgent";
				MASS_base::log(convert.str());
			}
			// agent instanstantiation and initialization
			Agent *newAgent = (Agent *)(agentsDllClass->instantiate(argument));

			if (printOutput == true) {
				convert.str("");
				convert << " newAgent[" << localPopulation << "] = " << (void *)newAgent;
				MASS_base::log(convert.str());
			}

			newAgent->agentsHandle = handle;
			newAgent->placesHandle = placesHandle;
			newAgent->agentId = currentAgentId++;
			newAgent->parentId = -1; // no parent
			newAgent->place = curPlace;

			for (int index = 0; index < int(curPlace->index.size()); index++)
				newAgent->index.push_back(curPlace->index[index]);
			newAgent->alive = true;

			// store this agent in the bag of agents
			agentsDllClass->agents->push_back(newAgent);

			// TODO: register newAgent into curPlace
			curPlace->agents.push_back((MObject *)newAgent);

			//newAgent.onCreation();
			//curPlace.onArrival();
			//newAgent.onArrival();
			addOne(curPlace);
		}
		if (printOutput) {
			convert.str("");
			convert << "Current outmessage = " << *(int*)curPlace->outMessage << std::endl;
			MASS_base::log(convert.str());
		}

	}
	delete protoAgent;
}

Agents_base::~Agents_base() {
}

/**
 *
 * @param functionId
 * @param argument
 * @param tid
 */
void Agents_base::callAll( int functionId, void *argument, int tid ) {

	//Set up the bag of agents
	DllClass *dllclass = MASS_base::dllMap[handle];

	//Create the bag for returning agents to be placed in
	vector<Agent *> *retBag = dllclass->retBag;
	int numOfOriginalVectors = Mthread::agentBagSize;
	ostringstream convert;

	while (true) {
		//Create the index for this iteration
		int myIndex;

		//Lock the index assignment so no two threads will receive the same value
		pthread_mutex_lock(&MASS_base::request_lock);

		/**/        //Thread checking
		if (printOutput == true) {
			convert.str("");
			convert << "Starting index value is: " << Mthread::agentBagSize;
			MASS_base::log(convert.str());
		}
		myIndex = Mthread::agentBagSize--;


		//Error Checking
		if (printOutput == true) {
			convert.str("");
			convert << "Thread[" << tid << "]: agent(" << myIndex << ") assigned";
			MASS_base::log(convert.str());
		}

		pthread_mutex_unlock(&MASS_base::request_lock);

		//Continue to run until the assigning index becomes negative
		//(in which case, we've run out of agents)
		if (myIndex > 0) {
			if (printOutput == true) {
				convert.str("");
			}
			//Lock the assignment and removal of agents
			pthread_mutex_lock(&MASS_base::request_lock);
			Agent *tmpAgent = dllclass->agents->back();
			dllclass->agents->pop_back();

			if (printOutput == true) {
				convert << "Thread [" << tid << "]: agent(" << tmpAgent << ")[" << myIndex << "] was removed ";
				convert << "fId = " << functionId << " argument " << argument;
				MASS_base::log(convert.str());
			}
			pthread_mutex_unlock(&MASS_base::request_lock);

			//Use the Agents' callMethod to have it begin running
			tmpAgent->callMethod(functionId, argument);

			if (printOutput == true) {
				convert.str("");
				convert << "Thread [" << tid << "]: (" << myIndex << ") has called its method; ";
				MASS_base::log(convert.str());
			}
			//Puth the now running thread into the return bag
			pthread_mutex_lock(&MASS_base::request_lock);
			retBag->push_back(tmpAgent);

			if (printOutput == true) {
				convert.str("");
				convert << "Thread [" << tid << "]: (" << myIndex << ") has been placed in the return bag; ";
				convert << "Current Agent Bag Size is: " << Mthread::agentBagSize;
				convert << " retBag.size = " << retBag->size();
				MASS_base::log(convert.str());
			}
			pthread_mutex_unlock(&MASS_base::request_lock);
		}
		//Otherwise, we are out of agents and should stop
		//trying to assign any more
		else {
			break;
		}
	}
	//Wait for the thread count to become zero
	Mthread::barrierThreads(tid);

	//Assign the new bag of finished agents to the old pointer for reuse
	if (tid == 0) {
		delete MASS_base::dllMap[handle]->agents;
		MASS_base::dllMap[handle]->agents = MASS_base::dllMap[handle]->retBag;
		Mthread::agentBagSize = numOfOriginalVectors;

		if (printOutput == true) {
			convert.str("");
			convert << "Agents_base:callAll: agents.size = " << MASS_base::dllMap[handle]->agents->size() << endl;
			convert << "Agents_base:callAll: agentsBagSize = " << Mthread::agentBagSize;
			MASS_base::log(convert.str());
		}
	}
}

/**
 * 
 * @param functionId
 * @param argument
 * @param arg_size
 * @param ret_size
 * @param tid
 */
void Agents_base::callAll(int functionId, void *argument, int arg_size,
	int ret_size, int tid) {

	//Set up the bag of agents
	DllClass *dllclass = MASS_base::dllMap[handle];

	//Create the bag for returning agents to be placed in
	vector<Agent *> *retBag = dllclass->retBag;
	int numOfOriginalVectors = Mthread::agentBagSize;
	ostringstream convert;

	while (true) {
		// create the index for this iteration
		int myIndex;

		// Lock the index assginment so no tow threads will receive the same index
		pthread_mutex_lock(&MASS_base::request_lock);

		// Thread checking
		if (printOutput == true) {
			convert.str("");
			convert << "Starting index value is: " << Mthread::agentBagSize;
			MASS_base::log(convert.str());
		}

		myIndex = Mthread::agentBagSize--; // myIndex == agentId + 1

		//Error Checking
		if (printOutput == true) {
			convert.str("");
			convert << "Thread[" << tid << "]: agent(" << myIndex << ") assigned";
			MASS_base::log(convert.str());
		}

		pthread_mutex_unlock(&MASS_base::request_lock);

		//While there are still indexes left, continue to grab and execute threads
		//When all are executing, place into vector and wait for them to finish
		if (myIndex > 0) {
			// compute where to store this agent's return value
			// note that myIndex = agentId + 1

			if (printOutput == true) {
				convert.str("");
				convert << "Thread[" << tid << "]: agent(" << myIndex << "): MASS_base::currentReturns  = " <<
					MASS_base::currentReturns
					<< " ret_size = " << ret_size;
				MASS_base::log(convert.str());
			}

			char *return_values = MASS_base::currentReturns + (myIndex - 1) * ret_size;

			if (printOutput == true) {
				convert.str("");
				convert << "Thread[" << tid << "]: agent(" << myIndex << "): return_values = " << return_values;
				convert << " dllclass->agents->size( ) = " << dllclass->agents->size();
				MASS_base::log(convert.str());
			}

			// Lock the assginment and removel of agents
			pthread_mutex_lock(&MASS_base::request_lock);

			Agent *tmpAgent = dllclass->agents->back();
			dllclass->agents->pop_back(); // check

			if (printOutput == true) {
				convert.str("");
				convert << "Thread [" << tid << "]: agent(" << myIndex << ") was removed = " << tmpAgent;
				convert << " dllclass->agents->size( ) = " << dllclass->agents->size();
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);
			//Use the Agents' callMethod to have it begin running

			if (printOutput == true) {
				convert.str("");
				convert << " agentsHandle = " << tmpAgent->agentsHandle;
				convert << " placesHandle = " << tmpAgent->placesHandle;
				convert << " agentId = " << tmpAgent->agentId;
				convert << " parentId = " << tmpAgent->parentId;
				convert << " place = " << tmpAgent->place;
				void *(Agent::*mfp)(int, void*) = &Agent::callMethod;
				convert << " callMethod = " << (void *)(tmpAgent->*mfp);
				MASS_base::log(convert.str());
			}
			//void *(Agent::*mfp)(int, void*) = &Agent::callMethod;
			//(tmpAgent->*mfp)( functionId, NULL );


			MASS_base::log("CHECK");
			void *retVal = tmpAgent->callMethod(functionId,
				(char *)argument + arg_size * (myIndex - 1));

			memcpy(return_values, retVal, ret_size); // get this callAll's return value

			if (printOutput == true) {
				convert << "Thread [" << tid << "]: (" << myIndex << ") has called its method; ";
			}
			//Puth the now running thread into the return bag
			pthread_mutex_lock(&MASS_base::request_lock);
			retBag->push_back(tmpAgent);

			if (printOutput == true) {
				convert << "Thread [" << tid << "]: (" << myIndex << ") has been placed in the return bag; ";
				convert << "Current Agent Bag Size is: " << Mthread::agentBagSize;
				convert << " retBag.size = " << retBag->size();
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);

		}
		//Otherwise, we are out of agents and should stop
		//trying to assign any more
		else {
			break;
		}
	}
	//Confirm all threads have finished
	Mthread::barrierThreads(tid);

	//Assign the new bag of finished agents to the old pointer for reuse
	if (tid == 0) {
		delete MASS_base::dllMap[handle]->agents;
		MASS_base::dllMap[handle]->agents = MASS_base::dllMap[handle]->retBag;
		Mthread::agentBagSize = numOfOriginalVectors;

		if (printOutput == true) {
			convert.str("");
			convert << "Agents_base:callAll: agents.size = " << MASS_base::dllMap[handle]->agents->size() << endl;
			convert << "Agents_base:callAll: agentsBagSize = " << Mthread::agentBagSize;
			MASS_base::log(convert.str());
		}

	}

}

/**
 *
 * @param src_index
 * @param dst_size
 * @param dest_dimension
 * @param dest_index
 */
void Agents_base::getGlobalAgentArrayIndex(vector<int> src_index,
	int dst_size[], int dest_dimension,
	int dest_index[]) {

	for (int i = 0; i < dest_dimension; i++) {
		dest_index[i] = src_index[i]; // calculate dest index

		if (dest_index[i] < 0 || dest_index[i] >= dst_size[i]) {
			// out of range
			for (int j = 0; j < dest_dimension; j++) {
				// all index must be set -1
				dest_index[j] = -1;
			}
			return;
		}
	}

}

/**
 *
 * @param tid
 */
void Agents_base::manageAll(int tid) {
	//Create the dllclass to access our agents from, out agentsDllClass for
	// agent instantiation, and our bag for Agent objects after they have
	// finished processing
	ostringstream convert;
	DllClass *dllclass = MASS_base::dllMap[handle];
	DllClass *agentsDllClass = new DllClass(className);
	Places_base *evaluatedPlaces = MASS_base::placesMap[placesHandle];
	vector<Agent *> *retBag = dllclass->retBag;
	// do collision free migration

	if (this->handle > 100) {
		while (true) {

			pthread_mutex_lock(&MASS_base::request_lock);
			int agentSize = dllclass->agents->size();
			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base::manageAll: Thread " << tid << " evaluated dllclass->agents->size() = "
					<< agentSize;
				MASS_base::log(convert.str());
			}
			if (agentSize == 0) {
				pthread_mutex_unlock(&MASS_base::request_lock);
				break;
			}

			//Grab the last agent and remove it for processing. Be sure to lock on
			// removal
			Agent *evaluationAgent = dllclass->agents->back();
			dllclass->agents->pop_back();

			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base::manageALL: Thread " << tid << " picked up "
					<< evaluationAgent->agentId;
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);

			// call spawn method
			spawnHelper(tid, evaluationAgent, agentsDllClass, retBag);
			//evaluationAgent->onCreation();

			// call kill method
			if (agentKilled(tid, evaluationAgent))
				continue;

			// move this evaluated agent to retBag
			pthread_mutex_lock(&MASS_base::request_lock);
			retBag->push_back(evaluationAgent);
			//evaluationAgent->place->onArrival();
			//evaluationAgent->onArrival();
			pthread_mutex_unlock(&MASS_base::request_lock);
		}
		// move all agents back to retBag
		moveAgentsBackToOriginalQueue(tid, moveRetBag2AgentsAndCreateNewRetBag);

		migrationHelperCollisionFree(tid, dllclass, agentsDllClass, evaluatedPlaces, retBag);

		// do normal migration
	}
	else {
		// Spawn, Kill, Migrate. Check in that order throughout the bag of agents
		// sequentially.
		while (true) {

			pthread_mutex_lock(&MASS_base::request_lock);
			int agentSize = dllclass->agents->size();
			if (agentSize == 0) {
				pthread_mutex_unlock(&MASS_base::request_lock);
				break;
			}

			//Grab the last agent and remove it for processing. Be sure to lock on
			// removal
			Agent *evaluationAgent = dllclass->agents->back();
			dllclass->agents->pop_back();

			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base::manageALL: Thread " << tid << " picked up "
					<< evaluationAgent->agentId;
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);

			// call spawn method
			spawnHelper(tid, evaluationAgent, agentsDllClass, retBag);
			//evaluationAgent->onCreation();
			// call kill method
			if (agentKilled(tid, evaluationAgent))
				continue;

			// call migrate method
			migrationHelper(tid, evaluationAgent, retBag, evaluatedPlaces);
			//evaluationAgent->place->onArrival();
			//evaluationAgent->onArrival();
		}
		manageAllCleanUp(tid, retBag, evaluatedPlaces, moveRetBag2Agents); // handle remote migration
	}
}

/**
 * spawns children of the agent pointed to by *evaluationAgent. It is called from
 * both manageAll( ) and migrationHelperCollisionFree( ).
 *
 * @param tid the id of a thread that invokes spawnHelper
 * @param evaluationAgent the pointer of an agent whose children will be spawned.
 * @agentsDllClass the dynamic linking library to instantiate a new child
 * @retBag the agent bag to store new children.
 */
void Agents_base::spawnHelper(int tid, Agent *evaluationAgent, DllClass *agentsDllClass, vector<Agent *> *retBag) {
	int argumentcounter = 0;
	//If the spawn's newChildren field is set to anything higher than zero
	//we need to create newChildren's worth of Agents in the current location.

	ostringstream convert;

	//Spawn() Check
	int childrenCounter = evaluationAgent->newChildren;

	if (printOutput == true) {
		convert.str("");
		convert << "spawnHelper: agent " << evaluationAgent->agentId
			<< "'s childrenCounter = " << childrenCounter;
		MASS_base::log(convert.str());
	}

	while (childrenCounter > 0) {
		if (printOutput == true) {
			convert.str("");
			convert << "Agent_base::manageALL: Thread " << tid
				<< " will spawn a child of agent " << evaluationAgent->agentId
				<< "...arguments.size( ) = "
				<< evaluationAgent->arguments.size()
				<< ", argumentcounter = " << argumentcounter;
			MASS_base::log(convert.str());
		}

		Agent *addAgent =
			// validate the correspondance of arguments and argumentcounter
			(int(evaluationAgent->arguments.size()) > argumentcounter) ?
			// yes: this child agent should recieve an argument.
			(Agent *)(agentsDllClass->instantiate(evaluationAgent->
				arguments[argumentcounter++])) :
			// no:  this child agent should not receive an argument.
			(Agent *)(agentsDllClass->instantiate(NULL));

		//Push the created agent into our bag for returns and update the counter
		//needed to keep track of our agents.

		//Lock here
		pthread_mutex_lock(&MASS_base::request_lock);
		retBag->push_back(addAgent);
		pthread_mutex_unlock(&MASS_base::request_lock);

		//Push the pointer copy into the current Agent's place location
		pthread_mutex_lock(&MASS_base::request_lock);
		evaluationAgent->place->agents.push_back((MObject *)addAgent);
		addOne(evaluationAgent->place); // for collision-free migration
		//evaluationAgent->place.onArrival();
		//evaluationAgent->onArrival();
		pthread_mutex_unlock(&MASS_base::request_lock);

		// initialize this child agent's attributes:
		addAgent->agentsHandle = evaluationAgent->agentsHandle;
		addAgent->placesHandle = evaluationAgent->placesHandle;
		addAgent->agentId = currentAgentId++;
		addAgent->index = evaluationAgent->index;
		addAgent->place = evaluationAgent->place;
		addAgent->parentId = evaluationAgent->agentId;

		//Decrement the newChildren counter once an Agent has been spawned
		evaluationAgent->newChildren--;
		childrenCounter--;

		if (printOutput == true) {
			convert.str("");
			convert << "Agent_base::manageALL spawnHelper( ): Thread " << tid
				<< " spawned a child of agent " << evaluationAgent->agentId
				<< " and put the child " << addAgent->agentId
				<< " child into retBag.";
			MASS_base::log(convert.str());
		}
	}
}

/**
 * kills the agent pointed to by *evaluationAgent. It is called from
 * both manageAll( ) and migrationHelperCollisionFree( ).
 *
 * @param tid the id of a thread that invokes agentKilled( ).
 * @param evaluationAgent the pointer of an agent that will be killed.
 */
bool Agents_base::agentKilled(int tid, Agent *evaluationAgent) {
	ostringstream convert;

	if (printOutput == true) {
		convert.str("");
		convert << "Agent_base::manageALL agentKilled( ): Thread " << tid
			<< " check " << evaluationAgent->agentId << "'s alive = "
			<< evaluationAgent->alive;
		MASS_base::log(convert.str());
	}
	if (evaluationAgent->alive == false) {

		//Get the place in which evaluationAgent is 'stored' in
		Place *evaluationPlace = evaluationAgent->place;

		// Move through the list of Agents to locate which to delete
		// Do so non-interruptively.
		pthread_mutex_lock(&MASS_base::request_lock);
		int evalPlaceAgents = evaluationPlace->agents.size();

		for (int i = 0; i < evalPlaceAgents; i++) {

			//Type casting used so we can compare agentId's
			MObject *comparisonAgent = evaluationPlace->agents[i];
			Agent *convertedAgent = static_cast<Agent *>(comparisonAgent);

			// Check the Id against the ID of the agent to be removed.
			// If it matches, remove it Lock
			if ((evaluationAgent->agentId == convertedAgent->agentId) &&
				(evaluationAgent->agentsHandle == convertedAgent->agentsHandle)) {
				evaluationPlace->agents.erase(evaluationPlace->agents.begin() + i);
				deleteOne(evaluationPlace); // for collision-free migration

				if (printOutput == true) {
					convert.str("");
					convert << "Agent_base::manageALL: Thread " << tid
						<< " deleted " << evaluationAgent->agentId
						<< " from place[" << evaluationPlace->index[0]
						<< "][" << evaluationPlace->index[1] << "]";
					MASS_base::log(convert.str());
				}
				break;
			}
		}

		pthread_mutex_unlock(&MASS_base::request_lock);

		//Delete the agent and its pointer to complete the removal
		//delete &evaluationAgent;
		delete evaluationAgent;
		return true;
	}
	return false; // should not be killed.
}

/**
 * Moves a given agent to a new place. It is called from manageAl( ) for original
 * collision-aware migration.
 *
 * @param tid the id of a thread that inovkes migrationHelper( ).
 * @param evaluationAgent the pointer of an agent that needs to migration to a new place.
 * @param retBag the bag of agents that should include this agent upon a local migration.
 * @param evaluatedPlaces the Places object to which a migrating agent is belonging.
 */
void Agents_base::migrationHelper(int tid, Agent *evaluationAgent, vector<Agent *> *retBag,
	Places_base *evaluatedPlaces) {

	ostringstream convert;
	//Iterate over all dimensions of the agent to check its location
	//against that of its place. If they are the same, return back.
	int agentIndex = evaluationAgent->index.size();
	int destCoord[agentIndex];

	// compute its coordinate
	getGlobalAgentArrayIndex(evaluationAgent->index, evaluatedPlaces->size,
		evaluatedPlaces->dimension, destCoord);

	if (printOutput == true) {
		convert.str("");
		convert << "pthread_self[" << pthread_self()
			<< "tid[" << tid << "]: calls from"
			<< "[" << evaluationAgent->index[0]
			<< "][" << evaluationAgent->index[1] << "]"
			<< " (destCoord[" << destCoord[0]
			<< "][" << destCoord[1] << "]"
			<< " evaluatedPlaces->size[" << evaluatedPlaces->size[0]
			<< "][" << evaluatedPlaces->size[1] << "]";
		MASS_base::log(convert.str());
	}

	if (destCoord[0] != -1) {
		// destination valid
		if (printOutput == true) {
			convert.str("");
			convert << "getGlobalLinearIndexFromGlobalArrayIndex: evaluatedPlace = " << evaluatedPlaces
				<< " destCoard = " << destCoord;
			MASS_base::log(convert.str());
		}
		int globalLinearIndex =
			evaluatedPlaces->getGlobalLinearIndexFromGlobalArrayIndex(destCoord,
				evaluatedPlaces->size,
				evaluatedPlaces->dimension);
		if (printOutput == true) {
			convert.str("");
			convert << " linear = " << globalLinearIndex
				<< " lower = " << evaluatedPlaces->lower_boundary
				<< " upper = " << evaluatedPlaces->upper_boundary << ")";
			MASS_base::log(convert.str());
		}

		// Should remove the pointer object in the place that points to
		// the migrting Agent
		Place *oldPlace = evaluationAgent->place;
		pthread_mutex_lock(&MASS_base::request_lock);
		// Scan old_place->agents to find this evaluationAgent's index.
		int oldIndex = -1;
		for (unsigned int i = 0; i < oldPlace->agents.size(); i++) {
			if (oldPlace->agents[i] == evaluationAgent) {
				oldIndex = i;
				break;
			}
		}
		if (oldIndex != -1) {
			oldPlace->agents.erase(oldPlace->agents.begin() + oldIndex);
			deleteOne(oldPlace); // for collsion-free migration
		}
		else {
			// should NOT happen
			if (printOutput == true) {
				convert.str("");
				convert << "evaluationAgent " << evaluationAgent->agentId
					<< " couldn't been found in the old place!";
				MASS_base::log(convert.str());
			}
			exit(-1);
		}
		if (printOutput == true) {
			convert.str("");
			convert << "evaluationAgent " << evaluationAgent->agentId
				<< " was removed from the oldPlace["
				<< oldPlace->index[0] << "]["
				<< oldPlace->index[1] << "]";
			MASS_base::log(convert.str());
		}
		pthread_mutex_unlock(&MASS_base::request_lock);

		if (globalLinearIndex >= evaluatedPlaces->lower_boundary &&
			globalLinearIndex <= evaluatedPlaces->upper_boundary) {
			// local destination

			// insert the migration Agent to a local destination place
			int destinationLocalLinearIndex
				= globalLinearIndex - evaluatedPlaces->lower_boundary;

			if (printOutput == true) {
				convert.str("");
				convert << "destinationLocalLinerIndex = "
					<< destinationLocalLinearIndex;
				MASS_base::log(convert.str());
			}

			DllClass *places_dllclass = MASS_base::dllMap[placesHandle];
			evaluationAgent->place
				= places_dllclass->places[destinationLocalLinearIndex];

			if (printOutput == true) {
				convert.str("");
				convert << "evaluationAgent->place = "
					<< evaluationAgent->place;
				MASS_base::log(convert.str());
			}

			pthread_mutex_lock(&MASS_base::request_lock);
			evaluationAgent->place->agents.push_back((MObject *)evaluationAgent);
			addOne(evaluationAgent->place); // for collision-free migration

			if (printOutput == true) {
				convert.str("");
				convert << "evaluationAgent " << evaluationAgent->agentId
					<< " was inserted into the destPlace["
					<< evaluationAgent->place->index[0] << "]["
					<< evaluationAgent->place->index[1] << "]";
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);

			//If not killed or migrated remotely, push Agent into the retBag
			pthread_mutex_lock(&MASS_base::request_lock);
			retBag->push_back(evaluationAgent);

			if (printOutput == true) {
				convert.str("");
				convert << "evaluationAgent " << evaluationAgent->agentId
					<< " was pushed back into retBag";
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);

		}
		else {
			// remote destination

			// find the destination node
			int destRank
				= evaluatedPlaces->getRankFromGlobalLinearIndex(globalLinearIndex);

			// create a request
			AgentMigrationRequest *request
				= new AgentMigrationRequest(globalLinearIndex, evaluationAgent);

			if (printOutput == true) {
				convert.str("");
				convert << "AgentMigrationRequest *request = " << request;
				MASS_base::log(convert.str());
			}

			// enqueue the request to this node.map
			pthread_mutex_lock(&MASS_base::request_lock);
			MASS_base::migrationRequests[destRank]->push_back(request);

			if (printOutput == true) {
				convert.str("");
				convert << "remoteRequest[" << destRank << "]->push_back:"
					<< " dst = " << globalLinearIndex;
				MASS_base::log(convert.str());
			}

			pthread_mutex_unlock(&MASS_base::request_lock);
		}
	}
	else {
		if (printOutput == true) {
			convert << " to destination invalid";
		}
	}
	if (printOutput == true) {
		MASS_base::log(convert.str());
	}
}

void Agents_base::migrationHelperCollisionFree(int tid, DllClass *dllclass,
	DllClass *agentsDllClass,
	Places_base *evaluatedPlaces,
	vector<Agent *> *retBag) {
	ostringstream convert;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			for (int z = -1; z <= 1; z++) {
				if (!((x != 0 && y == 0 && z == 0) ||
					(x == 0 && y != 0 && z == 0) ||
					(x == 0 && y == 0 && z != 0)))
					continue;
				// This implementation checks only North, East, South, West, Up, and Down
				if (printOutput == true) {
					convert.str("");
					convert << "migrationHelperCollisionFree: ";
					convert << "x = " << x << ", y = " << y << ", z = " << z;
					MASS_base::log(convert.str());
				}

				// Only thread 0 performs exchangeBoundary( ) with the width = 1
				if (tid == 0) {
					pthread_mutex_lock(&MASS_base::request_lock);
					dllclass->agentCounter = dllclass->agents->size();
					pthread_mutex_unlock(&MASS_base::request_lock);
					evaluatedPlaces->exchangeBoundary();
					if (printOutput == true)
						MASS_base::log("migrationHelperCollisionFree: exchangeBoundary completed");
				}
				Mthread::barrierThreads(tid);

				// now perform an agent migration for a given direction
				while (true) {
					pthread_mutex_lock(&MASS_base::request_lock);
					if (dllclass->agentCounter == 0) {
						// no more agents to move
						pthread_mutex_unlock(&MASS_base::request_lock);
						break;
					}
					// pick up the next agent to move
					Agent* evaluationAgent = dllclass->agents->back();
					dllclass->agents->pop_back();
					dllclass->agentCounter--;
					if (printOutput) {
						convert.str("");
						convert << "thread " << tid << " grabbed agent " << evaluationAgent->agentId << " for evaluation, ";
						convert << dllclass->agentCounter << " agents left to evaluate";
						MASS_base::log(convert.str());
					}
					pthread_mutex_unlock(&MASS_base::request_lock);

					// compute this agent's destination coordinate
					int agentIndex = evaluationAgent->index.size();
					int destCoord[agentIndex];
					getGlobalAgentArrayIndex(evaluationAgent->index, evaluatedPlaces->size,
						evaluatedPlaces->dimension, destCoord);

					if (destCoord[0] == -1) {
						// invalid destination
						// put in retbag
						if (printOutput)
							MASS_base::log("invalid destination");
						pthread_mutex_lock(&MASS_base::request_lock);
						dllclass->agents->insert(dllclass->agents->begin(), evaluationAgent);
						pthread_mutex_unlock(&MASS_base::request_lock);
						continue;
					}
					else if (calculateDirection(evaluationAgent, x, y, z) != true) {
						// agent isn't migrating in the current direction
						// put in original bag
						if (printOutput)
							MASS_base::log("not the current destination");
						pthread_mutex_lock(&MASS_base::request_lock);
						dllclass->agents->insert(dllclass->agents->begin(), evaluationAgent);
						pthread_mutex_unlock(&MASS_base::request_lock);
						continue;
					}
					else {
						if (printOutput)
							MASS_base::log("moving on the current destination");
						// check if destination is occupied
						// need to create a temp array because
						// destinationAgentSize needs an array not a vector
						int destIndex[agentIndex]; // must include a relative index
						for (int i = 0; i < agentIndex; i++) {
							// relative  = agent's abosolute index   - place's abusolute index
							destIndex[i] = evaluationAgent->index[i] - evaluationAgent->place->index[i];
						}

						int *outMessage = (int *)(evaluationAgent->place->getOutMessage(evaluationAgent->placesHandle, destIndex));
						int destinationAgentSize = (outMessage != NULL) ? *outMessage : 1;

						if (destinationAgentSize > 0) { // occupied!
						  // put in retBag because the agent cannot move
							if (printOutput) MASS_base::log("destination occupied");
							pthread_mutex_lock(&MASS_base::request_lock);
							dllclass->agents->insert(dllclass->agents->begin(), evaluationAgent);
							pthread_mutex_unlock(&MASS_base::request_lock);
							continue;
						}
						else if (destinationAgentSize < 0) { // garbage!
							MASS_base::log("Place has negative agent count, this should not occur.");
							exit(-1);
						}

						// destination is valid/unoccupied and in the current direction
						if (printOutput) MASS_base::log("destination unoccupied");
						int globalLinearIndex =
							evaluatedPlaces->getGlobalLinearIndexFromGlobalArrayIndex(destCoord,
								evaluatedPlaces->size,
								evaluatedPlaces->dimension);

						// Should remove the pointer object in the place that points to
						// the migrting Agent
						Place *oldPlace = evaluationAgent->place;

						pthread_mutex_lock(&MASS_base::request_lock);
						// Scan old_place->agents to find this evaluationAgent's index.
						int oldIndex = -1;
						for (unsigned int i = 0; i < oldPlace->agents.size();i++) {
							if (oldPlace->agents[i] == evaluationAgent) {
								oldIndex = i;
								break;
							}
						}

						if (oldIndex != -1) {
							oldPlace->agents.erase(oldPlace->agents.begin() + oldIndex);
							deleteOne(oldPlace);
							if (printOutput == true) MASS_base::log("this agent now departing from oldPlace");
							// agent being removed from oldPlace
						}
						else {
							// should not happen
							if (printOutput) {
								convert.str("");
								convert << "thread " << tid << ", agent " << evaluationAgent->agentId
									<< " couldn't been found in the old place. this should not occur";
								MASS_base::log(convert.str());
							}
							exit(-1);
						}
						pthread_mutex_unlock(&MASS_base::request_lock);

						if (globalLinearIndex >= evaluatedPlaces->lower_boundary &&
							globalLinearIndex <= evaluatedPlaces->upper_boundary) {

							if (printOutput) MASS_base::log("the destination is local.");
							// insert the migration Agent to a local destination place
							int destinationLocalLinearIndex
								= globalLinearIndex - evaluatedPlaces->lower_boundary;


							DllClass *places_dllclass = MASS_base::dllMap[placesHandle];
							evaluationAgent->place
								= places_dllclass->places[destinationLocalLinearIndex];


							pthread_mutex_lock(&MASS_base::request_lock);
							evaluationAgent->place->agents.push_back((MObject *)evaluationAgent);
							addOne(evaluationAgent->place);

							pthread_mutex_unlock(&MASS_base::request_lock);

							//If not killed or migrated remotely, push Agent into the retBag
							pthread_mutex_lock(&MASS_base::request_lock);
							retBag->push_back(evaluationAgent);

							pthread_mutex_unlock(&MASS_base::request_lock);
						}
						else {
							// do remote migration
							if (printOutput) MASS_base::log("the destination is remote.");

							// find the destination node
							int destRank
								= evaluatedPlaces->getRankFromGlobalLinearIndex(globalLinearIndex);

							// create a request
							AgentMigrationRequest *request
								= new AgentMigrationRequest(globalLinearIndex, evaluationAgent);

							// decrement old plave and remove agent
							// in original agents_base line 908 increment outmessage

							// enqueue the request to this node.map
							pthread_mutex_lock(&MASS_base::request_lock);
							MASS_base::migrationRequests[destRank]->push_back(request);

							pthread_mutex_unlock(&MASS_base::request_lock);
						}
					}
				} // end of while true
				manageAllCleanUp(tid, retBag, evaluatedPlaces, doNothing);
			} // end of z
		} // end of y
	} // end of x
	moveAgentsBackToOriginalQueue(tid, popAgentsFromRetBagAndPushThemToAgents);
}

bool Agents_base::calculateDirection(Agent *evaluationAgent, int x, int y, int z) {
	int dirX = evaluationAgent->index[0] - evaluationAgent->place->index[0];
	dirX = (dirX > 0) ? 1 : ((dirX == 0) ? 0 : -1);

	int dirY = (evaluationAgent->index.size() > 1) ?
		evaluationAgent->index[1] - evaluationAgent->place->index[1] : 0;
	dirY = (dirY > 0) ? 1 : ((dirY == 0) ? 0 : -1);

	int dirZ = (evaluationAgent->index.size() > 2) ?
		evaluationAgent->index[2] - evaluationAgent->place->index[2] : 0;
	dirZ = (dirZ > 0) ? 1 : ((dirZ == 0) ? 0 : -1);

	return (dirX == x && dirY == y && dirZ == z);
}

/**
 * Handles remote agent migration. It is called from both manageAll( ) and
 * migrationHelperCollisionFree( )
 *
 * @param tid the ide of a thread that invokes manageAllCleanUp( )
 * @param retBag all the agents processed so far
 * @param evaluatedPlaces the Places object to which this Agents_base is belonging
 * @param op moveRetBag2Agents in collision-aware and doNothing in collision-free
 * migration
 */
void Agents_base::manageAllCleanUp(int tid, vector<Agent *> *retBag, Places_base *evaluatedPlaces,
	moveAgentBackToOriginalQueueOperations op) {
	ostringstream convert;

	moveAgentsBackToOriginalQueue(tid, op);

	if (tid == 0) {

		if (printOutput == true) {
			convert.str("");
			convert << "tid[" << tid << "] now enters processAgentMigrationRequest";
			MASS_base::log(convert.str());
		}

		// the main thread spawns as many communication threads as the number of
		// remote computing nodes and let each invoke processAgentMigrationReq.

		// args to threads: rank, agentHandle, placeHandle, lower_boundary
		int comThrArgs[MASS_base::systemSize][4];
		pthread_t thread_ref[MASS_base::systemSize]; // communication thread id
		for (int rank = 0; rank < MASS_base::systemSize; rank++) {

			if (rank == MASS_base::myPid) // don't communicate with myself
				continue;

			// set arguments
			comThrArgs[rank][0] = rank;
			comThrArgs[rank][1] = handle; // agents' handle
			comThrArgs[rank][2] = evaluatedPlaces->handle;
			comThrArgs[rank][3] = evaluatedPlaces->lower_boundary;

			// start a communication thread
			if (pthread_create(&thread_ref[rank], NULL,
				Agents_base::processAgentMigrationRequest,
				comThrArgs[rank]) != 0) {
				MASS_base::log("Agents_base.manageAll: failed in pthread_create");
				exit(-1);
			}

			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base.manageAll will start processAgentMigrationRequest thread["
					<< rank << "] = " << thread_ref[rank];
				MASS_base::log(convert.str());
			}
		}

		// wait for all the communication threads to be terminated
		for (int rank = MASS_base::systemSize - 1; rank >= 0; rank--) {

			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base.manageAll will join processAgentMigrationRequest A thread["
					<< rank << "] = " << thread_ref[rank] << " myPid = " << MASS_base::myPid;
				MASS_base::log(convert.str());
			}
			if (rank == MASS_base::myPid) // don't communicate with myself
				continue;

			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base.manageAll will join processAgentMigrationRequest B thread["
					<< rank << "] = " << thread_ref[rank];
				MASS_base::log(convert.str());
			}

			pthread_join(thread_ref[rank], NULL);

			if (printOutput == true) {
				convert.str("");
				convert << "Agents_base.manageAll joined processAgentMigrationRequest C thread["
					<< rank << "] = " << thread_ref[rank];
				MASS_base::log(convert.str());
			}
		}
		localPopulation = MASS_base::dllMap[handle]->agents->size();

		if (printOutput == true) {
			convert.str("");
			convert << "Agents_base.manageAll completed: localPopulation = "
				<< localPopulation;
			MASS_base::log(convert.str());
		}
	}
	else {
		if (printOutput == true) {
			convert.str("");
			convert << "pthread_self[" << pthread_self()
				<< "] tid[" << tid << "] skips processAgentMigrationRequest";
			MASS_base::log(convert.str());
		}
	}
}

void Agents_base::moveAgentsBackToOriginalQueue(int tid, moveAgentBackToOriginalQueueOperations op) {
	//When while loop finishes in manageAll( ), all Agents reside in retBag.
	// Need to hook back up. 
	Mthread::barrierThreads(tid);

	ostringstream convert;
	//Assign the new bag of finished agents to the old pointer for reuse
	if (tid == 0) {
		if (printOutput == true) {
			convert.str("");
			convert << "Agents_base:manageAll: retBag.size = "
				<< MASS_base::dllMap[handle]->retBag->size()
				<< ", operation = " << op;
			MASS_base::log(convert.str());
		}

		switch (op) {
		case moveRetBag2Agents: // normal collision-aware migration
		  // simply move the retBag vector to agents
			delete MASS_base::dllMap[handle]->agents;
			MASS_base::dllMap[handle]->agents = MASS_base::dllMap[handle]->retBag;
			MASS_base::dllMap[handle]->retBag = NULL;
			Mthread::agentBagSize = MASS_base::dllMap[handle]->agents->size();
			break;

		case moveRetBag2AgentsAndCreateNewRetBag: // collision-free migration right after spawn
		  // move the retBag vector to agents and get prepared for a new vector for retBag
		  // so that collision-free migration can reuse retBag
			delete MASS_base::dllMap[handle]->agents;
			MASS_base::dllMap[handle]->agents = MASS_base::dllMap[handle]->retBag;
			MASS_base::dllMap[handle]->retBag = new vector<Agent *>;
			Mthread::agentBagSize = MASS_base::dllMap[handle]->agents->size();
			break;

		case popAgentsFromRetBagAndPushThemToAgents: // collison-free migration after all direction
		  // move each agent from the retBag vector to the agents vector
			while (MASS_base::dllMap[handle]->retBag->size() > 0) {
				MASS_base::dllMap[handle]->agents->push_back(MASS_base::dllMap[handle]->retBag->back());
				MASS_base::dllMap[handle]->retBag->pop_back();
			}
			delete MASS_base::dllMap[handle]->retBag;
			MASS_base::dllMap[handle]->retBag = NULL;
			break;

		case doNothing: // collsion-free migration for each direction
		  // postpone all the retBag operations after collision-free migration
			break;
		}

		if (printOutput == true) {
			convert.str("");
			convert << "Agents_base:manageAll: agents.size = "
				<< MASS_base::dllMap[handle]->agents->size() << endl;
			convert << "Agents_base:manageAll: agentsBagSize = "
				<< Mthread::agentBagSize;
			MASS_base::log(convert.str());
		}
	}

	// all threads must barrier synchronize here.
	Mthread::barrierThreads(tid);
}

/**
 * increases the number of agents residing this curPlace. This information is
 * stored in cur_lace->outMessage[0] when collision-free migration is used.
 *
 * @param curPlace the current place whose agent population is incremented by 1.
 */
void Agents_base::addOne(Place *curPlace) {
	if (this->handle < 100) // non collision-free migration is used.
		return;

	// collision-free migration is used.
	if (curPlace->outMessage == NULL) {
		curPlace->outMessage = new int;
		*(int*)curPlace->outMessage = 1;
		curPlace->outMessage_size = sizeof(int);
	}
	else {
		*(int*)curPlace->outMessage += 1;
	}
}

/**
 * decreases the number of agents residing this curPlace. This information is
 * stored in cur_lace->outMessage[0] when collision-free migration is used.
 *
 * @param curPlace the current place whose agent population is decremented by 1.
 */
void Agents_base::deleteOne(Place *curPlace) {
	if (this->handle < 100) // non collision-free migration is used.
		return;

	// collision-free migration is used.
	if (curPlace->outMessage == NULL) {
		// error!!
		MASS_base::log("deleteOne: curPlace->outMessage = NULL");
		exit(-1);
	}
	else {
		((int*)curPlace->outMessage)[(curPlace->outMessage_size - sizeof(int))]--;
	}
}

void *Agents_base::processAgentMigrationRequest(void *param) {
	int destRank = ((int *)param)[0];
	int agentHandle = ((int *)param)[1];
	int placeHandle = ((int *)param)[2];
	//  int my_lower_boundary = ( (int *)param )[3];

	vector<AgentMigrationRequest *> *orgRequest = NULL;
	ostringstream convert;

	if (printOutput == true) {
		convert.str("");
		convert << "pthread_self[" << pthread_self()
			<< "] rank[" << destRank << "]: starts processAgentMigrationRequest";
		MASS_base::log(convert.str());
	}

	// pick up the next rank to process
	orgRequest = MASS_base::migrationRequests[destRank];

	// for debugging
	pthread_mutex_lock(&MASS_base::request_lock);
	if (printOutput == true) {
		convert.str("");
		convert << "tid[" << destRank << "] sends an exhange request to rank: "
			<< destRank << " size() = " << orgRequest->size() << endl;
		MASS_base::log(convert.str());

		convert.str("");
		for (int i = 0; i < int(orgRequest->size()); i++) {
			convert << "send "
				<< (*orgRequest)[i]->agent << " to "
				<< (*orgRequest)[i]->destGlobalLinearIndex << endl;
		}
		MASS_base::log(convert.str());
	}
	pthread_mutex_unlock(&MASS_base::request_lock);

	// now compose and send a message by a child
	Message messageToDest(Message::AGENTS_MIGRATION_REMOTE_REQUEST,
		agentHandle, placeHandle, orgRequest);

	if (printOutput == true) {
		convert.str("");
		convert << "tid[" << destRank << "] made messageToDest to rank: "
			<< destRank;
		MASS_base::log(convert.str());
	}

	struct MigrationSendMessage rankNmessage;
	rankNmessage.rank = destRank;
	rankNmessage.message = &messageToDest;
	pthread_t thread_ref;
	pthread_create(&thread_ref, NULL, sendMessageByChild, &rankNmessage);

	// receive a message by myself
	Message *messageFromSrc = MASS_base::exchange.receiveMessage(destRank);

	// at this point, the message must be exchanged.
	pthread_join(thread_ref, NULL);

	if (printOutput == true) {
		convert.str("");
		convert << "pthread id = " << thread_ref
			<< "pthread_join completed for rank["
			<< destRank << "] and will delete messageToDest: " << &messageToDest;
		MASS_base::log(convert.str());

		convert.str("");
		convert << "Message Deleted";
		MASS_base::log(convert.str());
	}

	// process a message
	vector<AgentMigrationRequest *> *receivedRequest
		= messageFromSrc->getMigrationReqList();

	int agentsHandle = messageFromSrc->getHandle();
	int placesHandle = messageFromSrc->getDestHandle();
	Places_base *dstPlaces = MASS_base::placesMap[placesHandle];
	DllClass *agents_dllclass = MASS_base::dllMap[agentsHandle];
	DllClass *places_dllclass = MASS_base::dllMap[placesHandle];

	if (printOutput == true) {
		convert.str("");
		convert << "request from rank[" << destRank << "] = " << receivedRequest;
		convert << " size( ) = " << receivedRequest->size();
		MASS_base::log(convert.str());
	}

	// retrieve agents from receiveRequest
	while (receivedRequest->size() > 0) {
		AgentMigrationRequest *request = receivedRequest->back();
		receivedRequest->pop_back();
		int globalLinearIndex = request->destGlobalLinearIndex;
		Agent *agent = request->agent;

		// local destination
		int destinationLocalLinearIndex
			= globalLinearIndex - dstPlaces->lower_boundary;

		if (printOutput == true) {
			convert << " dstLocal = " << destinationLocalLinearIndex << endl;
		}

		Place *dstPlace =
			(Place *)(places_dllclass->places[destinationLocalLinearIndex]);

		// push this agent into the place and the entire agent bag.
		agent->place = dstPlace;
		agent->index = dstPlace->index; // inserted by Fukuda on 6/28/16
		pthread_mutex_lock(&MASS_base::request_lock);
		dstPlace->agents.push_back(agent);
		(MASS_base::agentsMap[agent->agentsHandle])->addOne(dstPlace);
		agents_dllclass->agents->push_back(agent);
		pthread_mutex_unlock(&MASS_base::request_lock);

		delete request;
	}

	if (printOutput == true) {
		convert.str("");
		convert << "pthread_self[" << pthread_self()
			<< "] retreive agents from rank[" << destRank << "]complated";
		MASS_base::log(convert.str());
	}
	pthread_exit(NULL);
	return NULL;
}

void *Agents_base::sendMessageByChild(void *param) {
	int rank = ((struct MigrationSendMessage *) param)->rank;

	ostringstream convert;
	if (printOutput == true) {
		convert << "pthread_self[" << pthread_self()
			<< "] sendMessageByChild to " << rank << " starts";
		MASS_base::log(convert.str());
	}

	Message *message = (Message *)((struct MigrationSendMessage *) param)->message;
	MASS_base::exchange.sendMessage(rank, message);

	if (printOutput == true) {
		convert.str("");
		convert << "pthread_self[" << pthread_self()
			<< "] sendMessageByChild to " << rank << " finished";
		MASS_base::log(convert.str());
	}
	pthread_exit(NULL);
	return NULL;
}

virtual void Agents_base::onArrival(){}
virtual void Agents_base::onCreation(){}
virtual void Agents_base::onDeparture(){}
