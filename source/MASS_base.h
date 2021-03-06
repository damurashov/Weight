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

#ifndef MASS_BASE_H
#define MASS_BASE_H

#include <fstream>   // ofstream
#include <iostream>  // cerr cout endl
#include <map>
#include <unordered_map>
#include <string>  // string
#include "Agents_base.h"
#include "DllClass.h"
#include "ExchangeHelper.h"
#include "Message.h"
#include "Mthread.h"  // pthread_t
#include "Places_base.h"
//#include "GraphPlaces.h"
#include "RemoteExchangeRequest.h"
#define CUR_SIZE 256
#define MASS_LOGS "MASS_logs"

using namespace std;

/**
 *
 */
class MASS_base {
    friend class MProcess;
    friend class Places_base;
    friend class Agents_base;
    friend class Message;
    friend class Place;
    friend class VertexPlace;

   public:
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     *-----------Elias --> Added for Graph Fewatures ----------------------------------------*/
    static void reinitializeMap();
    static unordered_map<int, unordered_map<std::string,int>*> distributed_map; //collection of vertetices
    static std::unordered_map<string, int>* getDistributedMap(int handle);
    /*---------------------------------------------------------------------------------------
     *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    static void initMASS_base(const char *name, int myPid, int nProc, int port);
    static bool initializeThreads(int nThr);
    static void log(string msg);
    static int getCores();
    static int getMyPid() { return myPid; };

    static Places_base *getCurrentPlaces() { return currentPlaces; };
    static Places_base *getDestinationPlaces() { return destinationPlaces; };
    static Agents_base *getCurrentAgents() { return currentAgents; };

    static int getCurrentFunctionId() { return currentFunctionId; };
    static void *getCurrentArgument() { return currentArgument; };
    static int getCurrentArgSize() { return currentArgSize; };
    static int getCurrentRetSize() { return currentRetSize; };
    static Message::ACTION_TYPE getCurrentMsgType() { return currentMsgType; };
    static vector<int *> *getCurrentDestinations() {
        return currentDestinations;
    };
    static vector<pthread_t> threads;  // including main and child threads
    static void setHosts(vector<string> host_args);
    static void showHosts();
    static int requestCounter;
  
    //make public for convinience 
    static int myPid; 
    static int systemSize;
    static vector<string> hosts;
    static vector<string> getHosts(){return hosts;};
    static string getMyHostName(){return hostName;};


   protected:
    static int MASS_PORT;
    static bool INITIALIZED;
    static char CUR_DIR[CUR_SIZE];
    static string hostName;                    // my local host name
    //static int myPid;                          // my pid or rank
    //static int systemSize;                     // # processes
    static ofstream logger;                    // logger
    //static vector<string> hosts;               // all host names
    static map<int, Places_base *> placesMap;  // a collection of Places
    static map<int, Agents_base *> agentsMap;  // a collection of Agents
    static map<int, DllClass *> dllMap;        // a collection of DllClasses/palce
    static vector<vector<RemoteExchangeRequest *> *> remoteRequests;
    static vector<vector<AgentMigrationRequest *> *> migrationRequests;
    static Places_base *currentPlaces;
    static Places_base *destinationPlaces;
    static int currentFunctionId;
    static void *currentArgument;
    static int currentArgSize;
    static int currentRetSize;
    static char *currentReturns;
    static vector<int *> *currentDestinations;
    static Message::ACTION_TYPE currentMsgType;
    static ExchangeHelper exchange;
    static Agents_base *currentAgents;

   private:
    static pthread_mutex_t log_lock;
    static pthread_mutex_t request_lock;
};

#endif
