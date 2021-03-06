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

#include "Mthread.h"
#include <sstream>  // ostringstream
#include "Agents_base.h"
#include "MASS_base.h"  // MASS_base::log( )
#include "Places_base.h"

// Used to toggle output for Mthread
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

/**
 * Mutex lock to protect critical sections of code from unpredictable/error
 * situations arising from multi-threaded applications using shared resources
 * (Thread interruption, interleaving, state changes, etc)
 */
pthread_mutex_t Mthread::lock;
/**
 * Pre-defined condition to signal when barrier is ready (thread can continue)
 */
pthread_cond_t Mthread::barrier_ready;
/**
 * Pre-defined condition to signal when barrier has completed (not used)
 */
pthread_cond_t Mthread::barrier_finished;
/**
 * Count of threads awaiting resolution for barrier synchronization
 */
int Mthread::barrier_count;
/**
 * The current status/state of this Mthread
 */
Mthread::STATUS_TYPE Mthread::status;
/**
 * Storage for the ID of this Mthread instance
 */
int Mthread::threadCreated;
/**
 * Number of Agents in simulation space
 */
int Mthread::agentBagSize;

/**
 * This method initalizes class members, in preparation of running a particular
 * threaded task.
 */
void Mthread::init() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&barrier_ready, NULL);
    pthread_cond_init(&barrier_finished, NULL);
    status = STATUS_READY;
    barrier_count = 0;
}

/**
 * This method begins the run process for a threaded task in MASS (Mthread).
 * Effectively, it starts a loop where the Mthread evaluates the next task to
 * perform, performs it, and then waits for other threads to complete before
 * moving on to the next task.
 *
 * @param threadId  a reference to the address that contains the thread ID for
 *                  the actual Mthread instance to run
 * @return          currently returns NULL when complete, can/should be adapted
 *                  to pass through (return) values associated with relevant
 *                  call (e.g. - callAll())
 */

void *Mthread::run(void *param) {
    int tid = *(int *)param;
    threadCreated = tid;

    // breath message
    ostringstream convert;
    if (printOutput == true) {
        convert << "Mthread[" << tid << "] inovked";
        MASS_base::log(convert.str());
    }

    // the followign variables are used to call callAll( )
    Places_base *places = NULL;
    Places_base *destinationPlaces = NULL;
    Agents_base *agents = NULL;

    int functionId = 0;
    void *argument = NULL;
    int arg_size = 0;
    int ret_size = 0;
    Message::ACTION_TYPE msgType = Message::EMPTY;
    vector<int *> *destinations = NULL;

    bool running = true;
    while (running) {
        // wait for a new command
        pthread_mutex_lock(&lock);
        if (status == STATUS_READY) pthread_cond_wait(&barrier_ready, &lock);

        // wake-up message
        if (printOutput == true) {
            convert.str("");
            convert << "Mthread[" << tid << "] woken up";
            MASS_base::log(convert.str());
        }

        pthread_mutex_unlock(&lock);

        // perform each task
        switch (status) {
            case STATUS_READY:
                if (printOutput == true)
                    MASS_base::log("Mthread reached STATUS_READY in switch");
                exit(-1);
                break;
            case STATUS_TERMINATE:
                running = false;
                break;
            case STATUS_CALLALL:
                places = MASS_base::getCurrentPlaces();
                functionId = MASS_base::getCurrentFunctionId();
                argument = MASS_base::getCurrentArgument();
                arg_size = MASS_base::getCurrentArgSize();
                msgType = MASS_base::getCurrentMsgType();
                ret_size = MASS_base::getCurrentRetSize();

                if (printOutput == true) {
                    convert.str("");
                    convert << "Mthread[" << tid << "] works on CALLALL:"
                            << " placese = " << (void *)places
                            << " functionId = " << functionId
                            << " argument = " << argument
                            << " arg_size = " << arg_size
                            << " msgType = " << msgType
                            << " ret_size = " << ret_size;
                    MASS_base::log(convert.str());
                }

                if (msgType == Message::PLACES_CALL_ALL_VOID_OBJECT) {
                    // cerr << "Mthread[" << tid << "] call all void object" <<
                    // endl;
                    places->callAll(functionId, argument, tid);
                } else {
                    // cerr << "Mthread[" << tid << "] call all return object"
                    // << endl;
                    places->callAll(functionId, argument, arg_size, ret_size,
                                    tid);
                }
                break;

            case STATUS_EXCHANGEALL:
                if (printOutput == true) {
                    convert.str("");
                    convert << "Mthread[" << tid << "] works on EXCHANGEALL";
                    MASS_base::log(convert.str());
                }

                places = MASS_base::getCurrentPlaces();
                functionId = MASS_base::getCurrentFunctionId();
                destinationPlaces = MASS_base::getDestinationPlaces();
                destinations = MASS_base::getCurrentDestinations();

                places->exchangeAll(destinationPlaces, functionId, destinations,
                                    tid);
                break;

            case STATUS_AGENTSCALLALL:
                agents = MASS_base::getCurrentAgents();
                functionId = MASS_base::getCurrentFunctionId();
                argument = MASS_base::getCurrentArgument();
                arg_size = MASS_base::getCurrentArgSize();
                msgType = MASS_base::getCurrentMsgType();
                ret_size = MASS_base::getCurrentRetSize();

                if (printOutput == true) {
                    convert.str("");
                    convert << "Mthread[" << tid << "] works on AGENST_CALLALL:"
                            << " agents = " << (void *)agents
                            << " functionId = " << functionId
                            << " argument = " << argument
                            << " arg_size = " << arg_size
                            << " msgType = " << msgType
                            << " ret_size = " << ret_size;
                    MASS_base::log(convert.str());
                }
                if (msgType == Message::AGENTS_CALL_ALL_VOID_OBJECT) {
                    // cerr << "Mthread[" << tid << "] call all agent void
                    // object" << endl;
                    agents->callAll(functionId, argument, tid);
                } else {
                    // cerr << "Mthread[" << tid << "] call all agents return
                    // object" << endl;
                    agents->callAll(functionId, argument, arg_size, ret_size,
                                    tid);
                }
                break;

            case STATUS_MANAGEALL:

                // Get agents to be called with Manageall
                agents = MASS_base::getCurrentAgents();

                // Send logging message
                if (printOutput == true) {
                    convert.str("");
                    convert << "Mthread[" << tid << "] works on MANAGEALL:"
                            << " agents = " << (void *)agents;
                    MASS_base::log(convert.str());
                }

                // Sent message for manageall
                agents->manageAll(tid);

                break;
        }

        // barrier
        barrierThreads(tid);
    }

    // last message
    if (printOutput == true) {
        convert.str("");
        convert << "Mthread[" << tid << "] terminated";
        MASS_base::log(convert.str());
    }
    return NULL;
}
/**
 * This method allows classes to set the status (STATUS_TYPE) value for
 * this thread. This is used to signal to threads when they can begin
 * work, what work to perform, and when to terminate.
 *
 * @param new_status  status to set for this Mthread instance
 */
void Mthread::resumeThreads(STATUS_TYPE new_status) {
    pthread_mutex_lock(&lock);
    status = new_status;
    pthread_cond_broadcast(&barrier_ready);
    pthread_mutex_unlock(&lock);
}

/**
 * This method provides a way for Mthread objects in the same thread
 * group to synchronize between each other.
 *
 * @param tid the ID of the thread that is initiating barrier
 * synchronization
 */
void Mthread::barrierThreads(int tid) {
    static int barrier_phases = 0;

    pthread_mutex_lock(&lock);
    if (++barrier_count < int(MASS_base::threads.size())) {
        ostringstream convert;
        if (printOutput == true) {
            convert << "tid[" << tid
                    << "] waiting: barrier = " << barrier_phases;
            MASS_base::log(convert.str());
        }

        pthread_cond_wait(&barrier_ready, &lock);
    } else {
        barrier_count = 0;
        status = STATUS_READY;
        ostringstream convert;
        if (printOutput == true) {
            convert << "tid[" << tid
                    << "] woke up all: barrier = " << barrier_phases;
            MASS_base::log(convert.str());
        }

        barrier_phases++;
        pthread_cond_broadcast(&barrier_ready);
    }
    pthread_mutex_unlock(&lock);
}
