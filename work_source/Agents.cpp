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
 The following acknowledgment shall be used where appropriate in publications,
 presentations, etc.: © 2014-2015 University of Washington. MASS was developed
 by Computing and Software Systems at University of Washington Bothell. THE
 SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Agents.h"
#include <iostream>
#include <sstream>  // for ostringstream
#include "MASS.h"
#include "Message.h"

// Used to toggle output for Agents.cpp
const bool printOutput = false;
// const bool printOutput = true;

Agents::Agents(int handle, string className, void *argument, int argument_size,
               Places *places, int initPopulation)
    : Agents_base(handle, className, argument, argument_size,
                  places->getHandle(), initPopulation),
      localAgents(new int[MASS_base::systemSize]) {
    init_master(argument, argument_size);
}

void Agents::init_master(void *argument, int argument_size) {
    // check if MASS_base::hosts is empty (i.e., Places not yet created)
    if (MASS_base::hosts.empty()) {
        cerr << "Agents(" << className << ") can't be created without Places!!"
             << endl;
        exit(-1);
    }

    // create a new list for message
    Message *m = new Message(Message::AGENTS_INITIALIZE, initPopulation, handle,
                             placesHandle, className, argument, argument_size);

    // send a AGENT_INITIALIZE message to each slave
    for (int i = 0; i < int(MASS::mNodes.size()); i++) {
        MASS::mNodes[i]->sendMessage(m);
        ostringstream convert;

        if (printOutput == true) {
            cerr << "printOutput = " << boolalpha << printOutput << endl;
            convert << "AGENT_INITIALIZE sent to " << i;
            MASS_base::log(convert.str());
        }
    }
    delete m;

    // Synchronized with all slave processes
    MASS::barrier_all_slaves(localAgents);
    localAgents[0] = localPopulation;

    total = 0;
    for (int i = 0; i < MASS_base::systemSize; i++) {
        total += localAgents[i];
        // for debugging

        if (printOutput == true) {
            cerr << "rank[" << i
                 << "]'s local agent population = " << localAgents[i] << endl;
        }
    }

    // register this agents in the places hash map
    MASS_base::agentsMap.insert(
        map<int, Agents_base *>::value_type(handle, this));
}

void Agents::callAll(int functionId) {
    ca_setup(functionId, NULL, 0, 0, Message::AGENTS_CALL_ALL_VOID_OBJECT);
}

void *Agents::callAll(int functionId, void *argument, int arg_size,
                      int ret_size) {
    return ca_setup(functionId, argument, arg_size, ret_size,
                    Message::AGENTS_CALL_ALL_RETURN_OBJECT);
}

void Agents::callAll(int functionId, void *argument, int arg_size) {
    ca_setup(functionId, argument, arg_size, 0,  // ret_size = 0
             Message::AGENTS_CALL_ALL_VOID_OBJECT);
}

void *Agents::ca_setup(int functionId, void *argument, int arg_size,
                       int ret_size, Message::ACTION_TYPE type) {
    // calculate the total number of agents
    total = 0;
    for (int i = 0; i < MASS_base::systemSize; i++) total += localAgents[i];

    // send a AGENTS_CALL_ALL message to each slave
    Message *m = NULL;
    for (int i = 0; i < int(MASS::mNodes.size()); i++) {
        // create a message
        if (type == Message::AGENTS_CALL_ALL_VOID_OBJECT)
            m = new Message(type, this->handle, functionId, argument, arg_size,
                            ret_size);
        else {
            // calculate argument position
            int arg_pos = 0;
            for (int dest = 0; dest <= i; dest++) {
                arg_pos += localAgents[dest];

                if (printOutput == true) {
                    cerr << "Agents.callAll: calc arg_pos = " << arg_pos
                         << " localAgents[" << dest
                         << "] = " << localAgents[dest] << endl;
                }
            }

            m = new Message(type, this->handle, functionId,
                            // argument body
                            (char *)argument + arg_size * arg_pos,
                            // argument size
                            arg_size * localAgents[i + 1], ret_size);

            if (printOutput == true) {
                cerr << "Agents.callAll: to rank[" << (i + 1)
                     << "] arg_size = " << arg_size
                     << " arg_pos = " << arg_size * arg_pos
                     << " arg_size = " << arg_size * localAgents[i + 1] << endl;
            }
        }

        // send it
        MASS::mNodes[i]->sendMessage(m);
        if (printOutput == true) {
            cerr << "AGENTS_CALL_ALL " << m->getAction() << " sent to " << i
                 << endl;

            cerr << "Bag Size is: " + MASS_base::dllMap[handle]->agents->size();
        }
        Mthread::agentBagSize = MASS_base::dllMap[handle]->agents->size();

        // make sure to delete it
        delete m;
    };

    // Check for correct behavior post-Agents_base implementation
    // retrieve the corresponding agents
    MASS_base::currentAgents = this;
    MASS_base::currentFunctionId = functionId;
    MASS_base::currentArgument = argument;
    MASS_base::currentArgSize = arg_size;
    MASS_base::currentMsgType = type;
    MASS_base::currentRetSize = ret_size;
    MASS_base::currentReturns =
        new char[total *
                 MASS_base::currentRetSize];  // prepare an entire return space
    // resume threads
    ostringstream convert;

    if (printOutput == true) {
        convert << "MASS_base::currentgAgents = " << MASS_base::currentAgents
                << endl;
        convert << "MASS_base::getCurrentgAgents = "
                << MASS_base::getCurrentAgents() << endl;
        MASS_base::log(convert.str());
    }

    MASS_base::dllMap[handle]->retBag = new vector<Agent *>;
    Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL);

    // callall implementatioin
    if (type == Message::AGENTS_CALL_ALL_VOID_OBJECT)
        Agents_base::callAll(functionId, argument,
                             0);  // 0 = the main thread id
    else
        Agents_base::callAll(functionId, (void *)argument, arg_size, ret_size,
                             0);

    // confirm all threads are done with agents.callAll
    Mthread::barrierThreads(0);
    localAgents[0] = localPopulation;

    // Synchronized with all slave processes by main thread.
    MASS::barrier_all_slaves(MASS_base::currentReturns, 0,
                             MASS_base::currentRetSize, localAgents);

    total = 0;
    for (int i = 0; i < MASS_base::systemSize; i++) {
        total += localAgents[i];
        // for debugging
        if (printOutput == true) {
            cerr << "rank[" << i
                 << "]'s local agent population = " << localAgents[i] << endl;
        }
    }

    return (void *)MASS_base::currentReturns;
}
/**
 * Calls callAll and manageAll functions consecutively without responding
 *  back to user application in each iteration.
 *
 * @param functionId : the function id that is executed
 * @param numberOfIterations : number of consecutive calls of callAll() and
 *        manageAll() functions
 */
void Agents::doAll(int functionId, int numberOfIterations) {
    for (int i = 0; i < numberOfIterations; i++) {
        callAll(functionId);
        manageAll();
    }
}
/**
 * Calls callAll and manageAll functions consecutively without responding
 *  back to user application in each iteration.
 *
 * @param functionId : the function id that is executed
 * @param argument : the argument to pass to each Agent
 * @param arg_size : the size of the argument array
 * @param numberOfIterations : number of consecutive calls of callAll() and
 *        manageAll() functions
 */
void *Agents::doAll(int functionId, void *argument, int arg_size,
                    int numberOfIterations, int ret_size) {
    void *returnPointer;
    for (int i = 0; i < numberOfIterations; i++) {
        returnPointer = callAll(functionId, argument, arg_size, ret_size);
        manageAll();
    }
    return returnPointer;
}
/**
 * Calls callAll and manageAll functions consecutively without responding
 *  back to user application in each iteration.
 *
 * @param functionIdList : the function id list that is executed
 * @param func_size : the size of the functionIdList array
 * @param argumentList : the arguments to pass to each Agent
 * @param arg_size : the size of the argument array
 * @param numberOfIterations : number of consecutive calls of callAll() and
 *        manageAll() functions
 */
void Agents::doAll(int *functionIdList, int func_size, void *argumentList,
                   int arg_size, int numberOfIterations) {
    for (int i = 0; i < numberOfIterations; i++) {
        void *argument =
            (argumentList != NULL && i < arg_size) ? (argumentList + i) : NULL;
        for (int j = 0; j < func_size; j++) {
            callAll(functionIdList[j], argument, arg_size);
            manageAll();
        }
    }
}
// If statements to check for each condition. No else, as an Agent
// can migrate and spawn at the same time.
// Iterate through all agents present.
void Agents::manageAll() { ma_setup(); }

void Agents::ma_setup() {
    // send an AGENTS_MANAGE_ALL message to each slave
    Message *m = NULL;
    for (int i = 0; i < int(MASS::mNodes.size()); i++) {
        // create a message
        m = new Message(Message::AGENTS_MANAGE_ALL, this->handle, 0);

        // send it
        MASS::mNodes[i]->sendMessage(m);

        // MThread Update
        Mthread::agentBagSize = MASS_base::dllMap[handle]->agents->size();

        // make sure to delete it
        delete m;
    }

    // retrieve the corresponding agents
    MASS_base::currentAgents = this;
    MASS_base::currentMsgType = Message::AGENTS_MANAGE_ALL;
    MASS_base::dllMap[handle]->retBag = new vector<Agent *>;

    // resume threads
    Mthread::resumeThreads(Mthread::STATUS_MANAGEALL);

    // callall implementatioin
    Agents_base::manageAll(0);  // 0 = the main thread id

    // confirm all threads are done with agents.callAll
    Mthread::barrierThreads(0);

    // Synchronized with all slave processes
    MASS::barrier_all_slaves(localAgents);
    localAgents[0] = localPopulation;

    total = 0;
    for (int i = 0; i < MASS_base::systemSize; i++) {
        total += localAgents[i];
        // for debugging
        if (printOutput == true) {
            cerr << "rank[" << i
                 << "]'s local agent population = " << localAgents[i] << endl;
        }
    }
}

int Agents::nAgents() {
    int nAgents = 0;
    for (int i = 0; i < MASS_base::systemSize; i++) nAgents += localAgents[i];
    return nAgents;
}
