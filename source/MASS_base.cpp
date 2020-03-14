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

#include "MASS_base.h"
#include <sstream>  // ostringstream

// Used to toggle output for MASS_base
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

// Allocate static space
int MASS_base::MASS_PORT = 0;
bool MASS_base::INITIALIZED = false;
char MASS_base::CUR_DIR[CUR_SIZE];
string MASS_base::hostName;
int MASS_base::myPid = 0;
int MASS_base::systemSize = 0;
ofstream MASS_base::logger;
vector<string> MASS_base::hosts;
vector<pthread_t> MASS_base::threads;
pthread_mutex_t MASS_base::log_lock;
pthread_mutex_t MASS_base::request_lock;
map<int, Places_base *> MASS_base::placesMap;
map<int, Agents_base *> MASS_base::agentsMap;
map<int, DllClass *> MASS_base::dllMap;
vector<vector<RemoteExchangeRequest *> *> MASS_base::remoteRequests;
vector<vector<AgentMigrationRequest *> *> MASS_base::migrationRequests;
int MASS_base::requestCounter;
Places_base *MASS_base::currentPlaces;
Places_base *MASS_base::destinationPlaces;
Agents_base *MASS_base::currentAgents;

int MASS_base::currentFunctionId;
void *MASS_base::currentArgument;
int MASS_base::currentArgSize;
int MASS_base::currentRetSize;
char *MASS_base::currentReturns;
vector<int *> *MASS_base::currentDestinations;
Message::ACTION_TYPE MASS_base::currentMsgType;
ExchangeHelper MASS_base::exchange;

void MASS_base::initMASS_base(const char *name, int myPid, int nProc,
                              int port) {
    // Initialize constants
    MASS_base::hostName = name;
    MASS_base::myPid = myPid;
    MASS_base::systemSize = nProc;
    MASS_base::MASS_PORT = port;
    MASS_base::currentPlaces = NULL;
    MASS_base::currentAgents = NULL;
    MASS_base::requestCounter = 0;
    pthread_mutex_init(&MASS_base::log_lock, NULL);
    pthread_mutex_init(&MASS_base::request_lock, NULL);

    // Get the current working directory
    bzero(MASS_base::CUR_DIR, CUR_SIZE);
    getcwd(MASS_base::CUR_DIR, CUR_SIZE);
    if (strlen(MASS_base::CUR_DIR) == 0) {
        if (printOutput == true) cerr << "getcwd failed" << endl;
        exit(-1);
    }
}

bool MASS_base::initializeThreads(int nThr) {
    //cerr << "Began initializeThreads()" << endl;
    if (INITIALIZED) {
        if (printOutput == true)
            cerr << "Error: the MASS.init is already initializecd" << endl;
        return false;
    }

    int cores = (nThr <= 0) ? getCores() : nThr;

    // all pthread_t structures
    threads.reserve(cores);
    threads.push_back(pthread_self());  // the main thread id

    pthread_t thread_ref;  // a temporary thread reference

    // initialize Mthread's static variables
    Mthread::init();

    // now launch child threads
    Mthread::threadCreated = 0;
    for (int i = 1; i < cores; i++) {
        if (pthread_create(&thread_ref, NULL, Mthread::run, &i) != 0) {
            log("pthread_create: error in MASS_base::initializeThreads");
            exit(-1);
        }
        threads.push_back(thread_ref);
        while (Mthread::threadCreated != i)
            ;  // busy wait
    }

    ostringstream convert;
    if (printOutput == true) {
        convert << "Initialized threads - # " << cores;
        log(convert.str());
    }

    INITIALIZED = true;
    return true;
}

void MASS_base::log(string msg) {
    pthread_mutex_lock(&log_lock);
    if (myPid == 0) {
        // The master node directly prints out the message to standard error.
        cerr << msg;// << endl;
    } else {
        // All the slave nodes prints out the message to CUR_DIR/MASS_logs/.
        if (!logger.is_open()) {
            // if not open, then open a logger.
            ostringstream convert;
            convert << MASS_LOGS << "/PID_" << myPid << "_" << hostName
                    << "result.txt";
            string filename = convert.str();
            logger.open(filename.c_str(), ofstream::out);
        }
        logger << msg << endl;
    }
    pthread_mutex_unlock(&log_lock);
}

void MASS_base::setHosts(vector<string> host_args) {
    if (!hosts.empty()) {
        // already initialized
        return;
    }

    // register all hosts including myself
    for (int i = 0; i < int(host_args.size()); i++) {
        if (printOutput == true) log(host_args[i]);
        hosts.push_back(host_args[i]);
    }

    // instantiate remoteRequests: vector< vector<RemoteExchangeReques*> >
    // as well as migrationRequests for the purpose of agent migration.
    for (int i = 0; i < systemSize; i++) {
        remoteRequests.push_back(new vector<RemoteExchangeRequest *>);
        migrationRequests.push_back(new vector<AgentMigrationRequest *>);
    }

    // establish inter-MASS connection
    exchange.establishConnection(systemSize, myPid, hosts, MASS_PORT);
}

void MASS_base::showHosts() {
    ostringstream convert;
    if (printOutput == true) {
        convert << "hosts....." << endl;
        for (int i = 0; i < int(hosts.size()); i++) {
            convert << "rank[" << i << "] = " << hosts[i] << endl;
        }
        MASS_base::log(convert.str());
    }
}

int MASS_base::getCores() {
    // TODO: to be implemented
    return 2;
}
