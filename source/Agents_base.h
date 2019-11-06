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

#ifndef AGENTS_BASE_H
#define AGENTS_BASE_H

#include <map>
#include <string>
#include <stdlib.h>
#include <sstream>
#include "Agent.h"
#include "DllClass.h"
#include "Places_base.h"

 // AgentId = 0..4294967295 i.e. 42.9 milion
#define MAX_AGENTS_PER_NODE 100000000 // 100 million

using namespace std;

/**
 * 
 */
enum moveAgentBackToOriginalQueueOperations {
	moveRetBag2Agents,
	moveRetBag2AgentsAndCreateNewRetBag,
	popAgentsFromRetBagAndPushThemToAgents,
	doNothing
};

class Agents_base {
	friend class MProcess;
public:
	Agents_base(int handle, string className, void *argument,
		int argument_size, int placesHandle, int initPopulation);
	~Agents_base();

	void callAll(int functionId, void *argument, int tid);
	void callAll(int functionId, void *argument, int arg_size,
		int ret_size, int tid);
	void manageAll(int tid);
	int nLocalAgents() { return localPopulation; };

	void getGlobalAgentArrayIndex(vector<int> src_index,
		int dst_size[], int dst_dimension,
		int dest_index[]);
	virtual void onArrival();
	virtual void onCreation();
	virtual void onDeparture();
	

protected:
	const int handle;
	const string className;
	const int placesHandle;

	int initPopulation;
	int localPopulation;
	unsigned int currentAgentId;
	void spawnHelper(int tid, Agent *evaluationAgent, DllClass *agentsDllClass, vector<Agent *> *retBag);
	bool agentKilled(int tid, Agent *evaluationAgent);
	void migrationHelper(int tid, Agent *evaluationAgent, vector<Agent *> *retBag, Places_base *evaluatedPlaces);
	void migrationHelperCollisionFree(int tid, DllClass *dllclass, DllClass *agentsDllClass,
		Places_base *evaluatedPlaces, vector<Agent *> *retBag);
	void manageAllCleanUp(int tid, vector<Agent *> *retBag, Places_base *evaluatedPlaces,
		moveAgentBackToOriginalQueueOperations op);
	void moveAgentsBackToOriginalQueue(int tid, moveAgentBackToOriginalQueueOperations op);
	bool calculateDirection(Agent *evalutionAgent, int x, int y, int z);
	void addOne(Place *curPlace);
	void deleteOne(Place *curPlace);

	static void *processAgentMigrationRequest(void *param);

	static void *sendMessageByChild(void *param);
	struct MigrationSendMessage {
		int rank;
		void *message;
	};
};

#endif
