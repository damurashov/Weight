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

#ifndef AGENT_H
#define AGENT_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include "MObject.h"
#include "Place.h"
using namespace std;

/**
 * Agent class
 * @param argument
 */
class Agent : MObject {
    friend class Agents_base;
    friend class Message;
    friend class AgentMigrationRequest;

   public:
    /**
     * Is the default constructor. A contiguous space of arguments is passed to
     * the constructor
     * @param argument
     */
    Agent(void *argument)
        : alive(true),
          newChildren(0),
          migratableData(NULL),
          migratableDataSize(0){};

    /**
     * Is called from Agents.callAll. It invokes the function specified with
     * functionId as passing arguments to this function. A user-derived Agent
     * class must implement this method.
     * @param functionId
     * @param argument
     * @return
     */
    virtual void *callMethod(int functionId, void *argument) = 0;

    /**
     * Destructor
     */
    ~Agent() {
        if (migratableData != NULL) free(migratableData);
    };

    /**
     * Returns the number of agents to initially instantiate on a place indexed
     * with coordinates[]. The maxAgents parameter indicates the number of
     * agents to create over the entire application. The argument size[] defines
     * the size of the "Place" matrix to which a given "Agent" class belongs.
     * The system-provided (thus default) map( ) method distributes agents over
     * places uniformly as in:
     *        maxAgents / size.length
     * The map( ) method may be overloaded by an application-specific method.
     * A user-provided map( ) method may ignore maxAgents when creating
     * agents.
     * @param initPopulation
     * @param size
     * @param index
     * @return
     */
    int map(int initPopulation, vector<int> size, vector<int> index,
            Place *curPlace);

    /**
     * Terminates the calling agent upon a next call to Agents.manageAll( ).
     * More specifically, kill( ) sets the "alive" variable false.
     */
    void kill() { alive = false; };

    /**
     *
     * @return id
     */
    int getDebugData() {
        int id = agentId;
        return id;
    }

    /**
     * modify the debug data of the agent
     */
    void setDebugData(int argument) {
        // currently empty
    }

   protected:
    bool migrate(int index, ...) { return true; };
    bool migrate(vector<int> index);
    void spawn(int nAgents, vector<void *> arguments, int arg_size);

    // void spawn( int nAgents, void *arguments , int arg_size );

    /**
     * Maintains this handle of the agents class to which this agent belongs.
     */
    int agentsHandle;

    /**
     * Maintains this handle of the agents class with which this agent is
     * associated.
     */
    int placesHandle;

    /**
     * Is this agent’s identifier. It is calculated as:
     * the sequence number * the size of this agent’s belonging
     * matrix + the index of the current place when all
     * places are flattened to a single dimensional array.
     */
    int agentId;

    /**
     * Is the identifier of this agent’s parent.
     */
    int parentId;

    /**
     * Points to the current place where this agent resides.
     */
    Place *place;

    /**
     * Is an array that maintains the coordinates of where this agent resides.
     * Intuitively, index[0], index[1], and index[2] correspond to coordinates
     * of x, y, and z, or those of i, j, and k.
     */
    vector<int> index;

    /**
     * Is true while this agent is active. Once it is set false, this agent is
     * killed upon a next call to Agents.manageAll( ).
     */
    bool alive;

    /**
     * Is the number of new children created by this agent upon a next call to
     * Agents.manageAll( ).
     */
    int newChildren;

    /**
     * Is an array of arguments, each passed to a different new child.
     */
    vector<void *> arguments;

    /**
     * Is a pointer to a user-allocated space that will be carried
     * with the agent when it migrates to a different space.
     * To be cast to a user-defined data type.
     */
    void *migratableData;

    /**
     * Indicates the size of the migratebleData space.
     */
    int migratableDataSize;
};

#endif
