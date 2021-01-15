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

#include "MProcess.h"
#include <dirent.h>     // opendir, closedir
#include <stdlib.h>     // atoi
#include <sys/stat.h>   // mkdir
#include <sys/types.h>  // mkdir, opendir, closedir
#include <unistd.h>     // chdir, sleep (just for debugging)
#include <sstream>      // ostringstream
#include "MASS_base.h"
#include "Mthread.h"
#include "FileParser.h"
#include <map>


// Toggles output for MProcess
#ifndef LOGGING
const bool printOutput = true;
#else
const bool printOutput = true;
#endif

MProcess::MProcess(char *name, int myPid, int nProc, int nThr, int port) {
    this->hostName = new string(name);
    this->myPid = myPid;
    this->nProc = nProc;
    this->nThr = nThr;
    this->port = port;
    MASS_base::initMASS_base(name, myPid, nProc, port);

    // Create a logger
    DIR *dir = NULL;
    if ((dir = opendir(MASS_LOGS)) == NULL)
        mkdir(MASS_LOGS, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    else
        closedir(dir);

    MASS_base::initializeThreads(nThr);
}

void MProcess::start() {
    MASS_base::log("MProcess started");

    // retrieve the client socket ipaddress
    int master_ip_size = 0;
    read(0, &master_ip_size, sizeof(int));
    char master_ip[master_ip_size + 1];
    bzero(master_ip, master_ip_size + 1);
    read(0, &master_ip, master_ip_size);

    // Create a bare socket to the master
    ostringstream convert;
    convert << "Create a socket to master_ip = " << master_ip << ", port " << port;
    MASS_base::log( convert.str( ) );
    Socket socket(port);
    do {
      this->sd = socket.getClientSocket(master_ip);
    } while ( this->sd == NULL_FD );
    if ( printOutput == true ) 
      MASS_base::log("Socket created");

    // Synchronize with the master node first.
    if ( printOutput == true ) 
      MASS_base::log("Synchronize with the master");
    sendAck();
    MASS_base::log("Synchronized");

    bool alive = true;
    while (alive) {
        // receive a new message from the master
        Message *m = receiveMessage();

        ostringstream convert;
        if (printOutput == true) {
	       convert << "A new message received: action = " << m->getAction() << endl;

	       MASS_base::log(convert.str());
       }

        // get prepared for the following arguments for PLACES_INITIALIZE
        vector<int> size;  // size[]
        vector<string> hosts;
        hosts.clear();
        int argument_size = 0;  // argument[argument_size];
        char *argument = NULL;
        Places_base *places = NULL;  // new Places
        Agents_base *agents = NULL;  // new Agents

        // retrieve an argument
        argument_size = m->getArgumentSize();
        argument = (argument_size > 0) ? new char[argument_size] : NULL;
        m->getArgument(argument);

    /*-----Elias--> initialization for Graph feature variables -------------*
    *-----------------------------------------------------------------------*/
        int result = -1;
        FILE_TYPE_ENUMS fileEnum ;//= NULL;
        bool edgeAdded;
        int numVertices;
        std::unordered_map<string, int> *dist_map = new unordered_map<string, int> ();  
        std::vector<string> *neighborVec = new vector<string>();
        std::vector<int> *v = new vector<int>();
        std::vector<string> vec;
        bool found = false; 
        int i = 0;
        int j = 0;  
        DllClass * dllclass = NULL;
        VertexPlace* vp = NULL; 
        map<int, DllClass *>::iterator it;   
    /*-----------------------------------------------------------------*/

        if (m != NULL) {
            switch (m->getAction()) {
                case Message::ACK:
                    sendAck();
                    break;

                case Message::EMPTY:
                    if (printOutput == true)
                        MASS_base::log("EMPTY received!!!!");
                    sendAck();
                    break;

                case Message::FINISH:
                    Mthread::resumeThreads(Mthread::STATUS_TERMINATE);
                    // confirm all threads are done with finish
                    Mthread::barrierThreads(0);
                    sendAck();
                    alive = false;
                    if (printOutput == true)
                        MASS_base::log("FINISH received and ACK sent");
                    break;

                case Message::PLACES_INITIALIZE:
                    if (printOutput == true)
                        MASS_base::log("PLACES_INITIALIZE received");
                    // create a new Places
                    size = m->getSize();

                    /* Places_base(int handle, string className, int boundary_width,
                                    void *argument, int argument_size, int dim, int size[]);*/
                    places = new Places_base(m->getHandle(), m->getClassname(),
                                        m->getBoundaryWidth(), argument,
                                        argument_size, size.size(), &size[0]);

                    for (int i = 0; i < int(m->getHosts().size()); i++)
                        hosts.push_back(m->getHosts()[i]);
                    // establish all inter-node connections within setHosts( )
                    MASS_base::setHosts(hosts);

                    MASS_base::placesMap.insert(
                        map<int, Places_base *>::value_type(m->getHandle(),
                                                            places));
                    sendAck();
                    if (printOutput == true)
                        MASS_base::log("PLACES_INITIALIZE completed and ACK sent");
                    break;

                case Message::PLACES_CALL_ALL_VOID_OBJECT:
                    if (printOutput == true)
                        MASS_base::log("PLACES_CALL_ALL_VOID_OBJECT received");

                    // retrieve the corresponding places
                    MASS_base::currentPlaces = MASS_base::placesMap[m->getHandle()];
                    MASS_base::currentFunctionId = m->getFunctionId();
                    MASS_base::currentArgument = (void *)argument;
                    MASS_base::currentArgSize = argument_size;
                    MASS_base::currentMsgType = m->getAction();

                    // resume threads to work on call all.
                    Mthread::resumeThreads(Mthread::STATUS_CALLALL);

                    // 3rd arg: 0 = the main thread id
                    MASS_base::currentPlaces->callAll(m->getFunctionId(), (void *)argument, 0);

                    // confirm all threads are done with places.callAll
                    Mthread::barrierThreads(0);

                    sendAck();
                    break;

                case Message::PLACES_CALL_ALL_RETURN_OBJECT:
                    if (printOutput == true)
                        MASS_base::log(
                            "PLACES_CALL_ALL_RETURN_OBJECT received");
                    // retrieve the corresponding places
                    MASS_base::currentPlaces = MASS_base::placesMap[m->getHandle()];
                    MASS_base::currentFunctionId = m->getFunctionId();
                    MASS_base::currentArgument = (void *)argument;

                    if (printOutput == true) {
                        ostringstream convert;
                        convert << "check 1 places_size = "
                                << MASS_base::currentPlaces->places_size;
                        MASS_base::log(convert.str());
                    }

                    MASS_base::currentArgSize = argument_size / MASS_base::currentPlaces->places_size;

                    if (printOutput == true) MASS_base::log("check 2");

                    MASS_base::currentRetSize = m->getReturnSize();
                    MASS_base::currentMsgType = m->getAction();
                    MASS_base::currentReturns =
                        new char[MASS_base::currentPlaces->places_size *
                                 MASS_base::currentRetSize];

                    // resume threads to work on call all.
                    Mthread::resumeThreads(Mthread::STATUS_CALLALL);

                    // 3rd arg: 0 = the main thread id

                    MASS_base::currentPlaces->callAll(
                        MASS_base::currentFunctionId,
                        MASS_base::currentArgument, MASS_base::currentArgSize,
                        MASS_base::currentRetSize, 0);

                    // confirm all threads are done with places.callAll with
                    // return objects
                    Mthread::barrierThreads(0);

                    if (printOutput == true) {
                        convert.str("");
                        convert << "PLACES_CALL_ALL_RETURN_OBJECT checking "
                                   "currentReturns";
                    }

                    sendReturnValues((void *)MASS_base::currentReturns,
                                     MASS_base::currentPlaces->places_size,
                                     MASS_base::currentRetSize);
                    delete MASS_base::currentReturns;
                    break;

                case Message::PLACES_CALL_SOME_VOID_OBJECT:
                    break;

                case Message::PLACES_EXCHANGE_ALL:
                    if (printOutput == true) {
                        convert.str("");
                        convert << "PLACES_EXCHANGE_ALL recweived handle = "
                                << m->getHandle()
                                << " dest_handle = " << m->getDestHandle();
                        MASS_base::log(convert.str());
                    }

                    // retrieve the corresponding places
                    MASS_base::currentPlaces = MASS_base::placesMap[m->getHandle()];
                    MASS_base::destinationPlaces = MASS_base::placesMap[m->getDestHandle()];
                    MASS_base::currentFunctionId = m->getFunctionId();
                    MASS_base::currentDestinations = m->getDestinations();

                    // reset requestCounter by the main thread
                    MASS_base::requestCounter = 0;

                    // for debug
                    MASS_base::showHosts();

                    // resume threads to work on call all.
                    Mthread::resumeThreads(Mthread::STATUS_EXCHANGEALL);

                    // exchangeall implementation
                    MASS_base::currentPlaces->exchangeAll(
                        MASS_base::destinationPlaces,
                        MASS_base::currentFunctionId,
                        MASS_base::currentDestinations, 0);

                    // confirm all threads are done with places.exchangeall.
                    Mthread::barrierThreads(0);
                    if (printOutput == true) MASS_base::log("barrier done");

                    sendAck();
                    if (printOutput == true)
                        MASS_base::log(
                            "PLACES_EXCHANGE_ALL completed and ACK sent");
                    break;

                case Message::PLACES_EXCHANGE_BOUNDARY:
                    if (printOutput == true) {
                        convert.str("");
                        convert << "PLACES_EXCHANGE_BOUNDARY received handle = "
                                << m->getHandle();
                        MASS_base::log(convert.str());
                    }

                    // retrieve the corresponding places
                    MASS_base::currentPlaces =
                        MASS_base::placesMap[m->getHandle()];

                    // for debug
                    MASS_base::showHosts();

                    // exchange boundary implementation
                    MASS_base::currentPlaces->exchangeBoundary();

                    sendAck();
                    if (printOutput == true)
                        MASS_base::log(
                            "PLACES_EXCHANGE_BOUNDARY completed and ACK sent");
                    break;

                case Message::PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
                case Message::PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:
                case Message::PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST:
                    break;

                case Message::AGENTS_INITIALIZE:
                    if (printOutput == true)
                        MASS_base::log("AGENTS_INITIALIZE received");

                    agents = new Agents_base(m->getHandle(), m->getClassname(),
                                             argument, argument_size,
                                             m->getDestHandle(),
                                             m->getAgentPopulation());

                    MASS_base::agentsMap.insert(
                        map<int, Agents_base *>::value_type(m->getHandle(),
                                                            agents));
                    sendAck(agents->localPopulation);
                    if (printOutput == true)
                        MASS_base::log(
                            "AGENTS_INITIALIZE completed and ACK sent");
                    break;

                case Message::AGENTS_CALL_ALL_VOID_OBJECT:
                    if (printOutput == true)
                        MASS_base::log("AGENTS_CALL_ALL_VOID_OBJECT received");
                    MASS_base::currentAgents =
                        MASS_base::agentsMap[m->getHandle()];
                    MASS_base::currentFunctionId = m->getFunctionId();
                    MASS_base::currentArgument = (void *)argument;
                    MASS_base::currentArgSize = argument_size;
                    MASS_base::currentMsgType = m->getAction();

                    Mthread::agentBagSize =
                        MASS_base::dllMap[m->getHandle()]->agents->size();
                    MASS_base::dllMap[m->getHandle()]->retBag =
                        new vector<Agent *>;

                    // resume threads to work on call all
                    Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL);

                    MASS_base::currentAgents->callAll(m->getFunctionId(),
                                                      (void *)argument, 0);

                    // confirm all threads are done with agents.callAll
                    Mthread::barrierThreads(0);
                    if (printOutput == true) MASS_base::log("barrier done");

                    sendAck(MASS_base::currentAgents->localPopulation);
                    break;

                case Message::AGENTS_CALL_ALL_RETURN_OBJECT:
                    if (printOutput == true)
                        MASS_base::log(
                            "AGENTS_CALL_ALL_RETURN_OBJECT received");
                    MASS_base::currentAgents =
                        MASS_base::agentsMap[m->getHandle()];
                    MASS_base::currentFunctionId = m->getFunctionId();
                    MASS_base::currentArgument = (void *)argument;
                    MASS_base::currentArgSize =
                        (MASS_base::currentAgents->localPopulation > 0)
                            ? argument_size /
                                  MASS_base::currentAgents->localPopulation
                            : 0;
                    MASS_base::currentRetSize = m->getReturnSize();
                    MASS_base::currentMsgType = m->getAction();
                    MASS_base::currentReturns =
                        new char[MASS_base::currentAgents->localPopulation *
                                 MASS_base::currentRetSize];

                    Mthread::agentBagSize =
                        MASS_base::dllMap[m->getHandle()]->agents->size();
                    MASS_base::dllMap[m->getHandle()]->retBag =
                        new vector<Agent *>;

                    // resume threads to work on call all with return objects
                    Mthread::resumeThreads(Mthread::STATUS_AGENTSCALLALL);

                    MASS_base::currentAgents->callAll(
                        MASS_base::currentFunctionId,
                        MASS_base::currentArgument, MASS_base::currentArgSize,
                        MASS_base::currentRetSize, 0);

                    // confirm all threads are done with agnets.callAll with
                    // return objects
                    Mthread::barrierThreads(0);
                    if (printOutput == true) MASS_base::log("barrier done");

                    sendReturnValues((void *)MASS_base::currentReturns,
                                     MASS_base::currentAgents->localPopulation,
                                     MASS_base::currentRetSize,
                                     MASS_base::currentAgents->localPopulation);

                    delete MASS_base::currentReturns;

                    break;
                case Message::AGENTS_MANAGE_ALL:
                    if (printOutput == true)
                        MASS_base::log("AGENTS_MANAGE_ALL received");
                    MASS_base::currentAgents =
                        MASS_base::agentsMap[m->getHandle()];

                    Mthread::agentBagSize =
                        MASS_base::dllMap[m->getHandle()]->agents->size();
                    MASS_base::dllMap[m->getHandle()]->retBag =
                        new vector<Agent *>;
                    Mthread::resumeThreads(Mthread::STATUS_MANAGEALL);

                    MASS_base::currentAgents->manageAll(
                        0);  // 0 = the main thread id

                    // confirm all threads are done with agents.manageAll.
                    Mthread::barrierThreads(0);
                    if (printOutput == true) {
                        convert.str("");
                        convert << "sendAck will send localPopulation = "
                                << MASS_base::currentAgents->localPopulation;
                        MASS_base::log(convert.str());
                    }

                    sendAck(MASS_base::currentAgents->localPopulation);

                    break;         
                            
            /* ----------------------------------------------------------------------------------------------
            *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            *+++++ Elias-->Added to support graph features ++++++++++++++++++++++++++++++++++++++++++++++++++

            *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
            *-----------------------------------------------------------------------------------------------*/                
     
                /*@param action         
                 * @param size
                 * @param handle                            
                 * @param classname      
                 * @param argument
                 * @param arg_size
                 * @param boundary_width
                 * @param hosts
                 * @ numVertices
                 * @fileType
                 * @filename*/
                case Message::PLACES_INITIALIZE_GRAPH_FROM_FILE:
                     if(printOutput){
                        MASS_base::log("PLACES_INITIALIZE_GRAPH_FROM_FILE message received");
                        convert << "handle = "<< m->getHandle() << ", classname = " << m->getClassname()
                        << ", boundary width = " << m->getBoundaryWidth() << ", dimension = " << m->getSize().size()
                        << ", filename = " << m->getFilename() << ", argument = " << (char*)argument << ", argument size = "
                        << argument_size << ", distributed_map size = " << (m->getDistributed_map())->size() << endl ;

                        MASS_base::log(convert.str());
                    }             
                       
                                     
                    dist_map = m->getDistributed_map();

                    //insert the map into MASS_base::distributed_map
                    MASS_base::distributed_map.insert({m->getHandle(), dist_map});

                    //read file Enum
                    fileEnum = FileParser::fromStringToEnum(m->getFileType());
                    places = new Places_base(m->getHandle(), m->getClassname(), m->getBoundaryWidth(),
                            m->getSize().size(), m->getFilename(), fileEnum,argument, argument_size);

                    for (int i = 0; i < int(m->getHosts().size()); i++)
                        hosts.push_back(m->getHosts()[i]);

                    // establish all inter-node connections within setHosts( )
                    MASS_base::setHosts(hosts);

                    MASS_base::placesMap.insert(map<int, Places_base *>::value_type(m->getHandle(), places));

                    sendAck();
                    
                    if (printOutput){
                        MASS_base::log("PLACES_INITIALIZE_GRAPH_FROM_FILE completed and ACK sent");
                    }

                    break;

                /*@param: action
                 *@param: size_vector
                 *@param: handle
                 *@param: className
                 *@param: argument
                 *@param: arg_size
                 *@param: boundary_width
                 *@param: hosts*/    
              case Message::PLACES_INITIALIZE_GRAPH_FROM_EMPTY:

                   if (printOutput == true)
                        MASS_base::log("PLACES_INITIALIZE_GRAPH_FROM_EMPTY received");
                    // create a new Places
                    size = m->getSize();
                    numVertices = size[0];
                    places = new Places_base(m->getHandle(), m->getClassname(),m->getBoundaryWidth(), 
                            size.size(), argument, argument_size, numVertices);


                    for (int i = 0; i < int(m->getHosts().size()); i++)
                        hosts.push_back(m->getHosts()[i]);
                    // establish all inter-node connections within setHosts( )
                    MASS_base::setHosts(hosts);

                    MASS_base::placesMap.insert(
                        map<int, Places_base *>::value_type(m->getHandle(),
                                                            places));
                    sendAck();
                    if (printOutput == true)
                        MASS_base::log("PLACES_INITIALIZE_GRAPH_FROM_EMPTY completed and ACK sent");
                    break;

                /*@param: action
                 *@param: handle
                 *@param: map<string, int>*/    
                case Message::GET_GRAPH_DISTRIBUTED_MAP: 
                     if(printOutput){
                        convert << " GET_GRAPH_DISTRIBUTED_MAP message received." << endl;
                        MASS_base::log(convert.str());
                     }

                     // static map<int, map<std::string,int>*> distributed_map; 
                      // retrieve the corresponding distributed_map from message
                     dist_map = m->getDistributed_map();

                     MASS_base:: distributed_map.insert({m->getHandle(),dist_map });

                     sendAck();

                     if(printOutput){
                        ostringstream convert;
                        convert << " GET_GRAPH_DISTRIBUTED_MAP--> message ACK sent. "
                        << " distributed_map size = " << m->getDistributed_map()->size() << endl;
                        MASS_base::log(convert.str());
                     }

                     break;   


              /* @param action
               * @param handle
               * @param functionId
               * @param argument
               * @param arg_size
               * @param ret_size*/
              case Message::MAINTENANCE_ADD_PLACE:
                    if(printOutput){
                        convert << " MProcess-->MAINTENANCE_ADD_PLACE message received." << endl;
                        MASS_base::log(convert.str());
                    }

                    // retrieve the corresponding places
                    
                    if(printOutput){
                        convert;
                        convert << "MAINTENANCE_ADD_PLACE--> handle = " + m->getHandle() <<  "places = "
                        << places << " argument = " << (char*) m->getArgumentPointer() << endl;
                        MASS_base::log(convert.str());
                    }
                    MASS_base::currentPlaces =  MASS_base::placesMap[m->getHandle()];
                    
                    MASS_base::currentPlaces->addPlaceLocally(m->getClassname(), 
                                                m->getArgumentPointer(), m->getArgumentSize());
                    sendAck(result);

                    if(printOutput){

                        ostringstream convert;
                        convert << "MAINTENANCE_ADD_PLACE-->Place added locally and ACK sent back." <<endl;
                        MASS_base::log(convert.str()); 
                    }
                    break; 

                   

                /*@handle 
                 *@vertexId
                 *@neighborId
                 *@weight*/
               case Message::MAINTENANCE_ADD_EDGE:
                     if(printOutput){
                        convert << "MAINTENANCE_ADD_EDGE message received." << endl;
                        MASS_base::log(convert.str());
                     }
                     // retrieve the corresponding places
                    MASS_base::currentPlaces =  MASS_base::placesMap[m->getHandle()];
                   
                    //bool addEdgeLocally(std::string vertexId, std::string neighborId, double weight);
                  edgeAdded =   MASS_base::currentPlaces->addEdgeLocally(m->getVertexId(), m->getNeighborId(), m->getweight());
                
                   //let the master know the edge is added or not
                   if(edgeAdded){
                    sendAck(1);
                   }
                   else{
                    sendAck(0);
                   }

                   if(printOutput){
                    ostringstream convert;
                    convert << "MAINTENANCE_ADD_EDGE message ACK sent." << endl;
                    MASS_base::log(convert.str());
                   }
                   break; 
                   

               /*@param action
                *@param initPopulation
                *@param handle
                *@param placeHandle
                *@param className
                *@param argument
                *@param argument_size*/
                case Message::MAINTENANCE_REMOVE_PLACE:
                     if(printOutput){
                        convert << "MAINTENANCE_REMOVE_PLACE message received to remove " 
                        << m->getClassname() << endl;
                        
                        MASS_base::log(convert.str());
                     }
                    // retrieve the corresponding places
                    MASS_base::currentPlaces =  MASS_base::placesMap[m->getHandle()];
                    /*NOTE: here m->getClassname() returns the vertex name to be removed,
                     *not the the class name !! refere how the message constructed in GraphPlaces*/
                    MASS_base::currentPlaces->removeVertexLocally(m->getClassname());

                    sendAck();
                    if(printOutput){
                        ostringstream convert;
                        convert << "MAINTENANCE_REMOVE_PLACE ACK sent. " << endl;
                        MASS_base::log(convert.str());
                    }

                     break;

                /*@handle 
                 *@vertexId
                 *@neighborId
                 *@weight*/    
                case Message::MAINTENANCE_REMOVE_EDGE:
                     if(printOutput){
                        convert << "MAINTENANCE_REMOVE_EDGE message received. " << endl;
                        MASS_base::log(convert.str());
                     }
                     // retrieve the corresponding places
                    MASS_base::currentPlaces =  MASS_base::placesMap[m->getHandle()];

                    //bool removeEdgeLocally(std::string vertexId, std::string neighborId);
                    MASS_base::currentPlaces->removeEdgeLocally(m->getVertexId(), m->getNeighborId());

                    sendAck();

                    if(printOutput){
                        ostringstream convert;
                        convert << "MAINTENANCE_REMOVE_EDGE ACK sent." << endl;
                        MASS_base::log(convert.str());
                    }

                    break;


                  /* @param action
                   * @param handle
                   * @param dummy  */  
                case Message::MAINTENANCE_REINITIALIZE:
                     if(printOutput){
                        convert << "MAINTENANCE_REINITIALIZE message received." << endl;
                        MASS_base::log(convert.str());
                     }

                    // retrieve the corresponding places
                    MASS_base::currentPlaces =  MASS_base::placesMap[m->getHandle()];
                   
                     MASS_base::currentPlaces->deleteAndRenitializeGraph();
                     
                    sendAck();

                    if(printOutput){
                        ostringstream convert;
                        convert << "MAINTENANCE_REINITIALIZE ACK sent." << endl;
                        MASS_base::log(convert.str());
                    }
                    break; 
                case Message::GET_VERTEXPLACE_FROM_VERTEXNAME:
                     if(printOutput){
                        MASS_base::log("GET_VERTEXPLACE_FROM_VERTEXNAME received");
                     }
                      
                    MASS_base::currentPlaces = MASS_base::placesMap[m->getHandle()];
                    places = MASS_base::getCurrentPlaces();
                    dist_map = places->getDistributedMap();
                    
                    /*ACTION_TYPE action,vector<int> *size, int handle, string classname, 
                        void *argument, int arg_size, int boundary_width, vector<string> *hosts)*/
                    if(dist_map == NULL){
                       m = new Message(Message::GET_VERTEXPLACE_FROM_VERTEXNAME, v, 0,"", NULL, 0, 0, neighborVec);
                        sendMessage(m);
                        break;
                    }

                    //Note: m->getClassname() returns the vertex name here: not a classname. 
                    if(dist_map->end() == dist_map->find(m->getClassname())){

                       m = new Message(Message::GET_VERTEXPLACE_FROM_VERTEXNAME, v, 0,"", NULL, 0, 0, neighborVec);
                        sendMessage(m);
                        break;
                    }

                     it = MASS_base::dllMap.find(m->getHandle());
                     if(it == MASS_base::dllMap.end()){
                        m = new Message(Message::GET_VERTEXPLACE_FROM_VERTEXNAME, v, 0,"", NULL, 0, 0, neighborVec);
                        sendMessage(m);
                        break;
                     }
                     
                     dllclass = it->second;
                     for(i = 0 ; i < (int)dist_map->size(); i++){
                        vp = (VertexPlace*) dllclass->places[i];
                        if(vp->getVertexName() == m->getClassname()){
                            vec = vp->getNeighbors();
                            neighborVec = &vec;
                            found = true;
                            break;
                        }
                     }
                    if(found){
                        m = new Message(Message::GET_VERTEXPLACE_FROM_VERTEXNAME, v, 0, m->getClassname(),
                            NULL, 0, 0, neighborVec);
                        sendMessage(m);
                        break;
                    }
                    else{

                        m = new Message(Message::GET_VERTEXPLACE_FROM_VERTEXNAME, v, 0,"", NULL, 0, 0, neighborVec);
                        sendMessage(m);
                    }
                    
                    break;


                /* @param action
                 * @param handle
                 * @param dummy*/
                /*case Message::MAINTENANCE_GET_PLACES:
                     if(printOutput){
                        ostringstream convert;
                        convert << " MAINTENANCE_GET_PLACES message received. " << endl;
                        MASS_base::log(convert.str());
                     }
                      // retrieve the corresponding places
                    MASS_base::currentPlaces =  MASS_base::placesMap[m->getHandle()];
                    grraphplaces = (GraphPlaces*)MASS_base::getCurrentPlaces();


                    /* ************************* serialization needed*************************
                     =========================================================================*/
                     /*break; */



                /*case Message::GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: //TODO implement this

                    /* ************************* serialization needed*************************
                     =========================================================================*/
                     /*break;*/
               

                //no need to implement this.
               case Message::MAINTENANCE_GET_PLACES_RESPONSE:               
                    break;
            }
            delete m;
        }
    }
}

void MProcess::sendAck() {
    Message *msg = new Message(Message::ACK);
    sendMessage(msg);
    delete msg;
}

void MProcess::sendAck(int localPopulation) {
    Message *msg = new Message(Message::ACK, localPopulation);
    ostringstream convert;
    if (printOutput == true) {
        convert << "msg->getAgentPopulation = " << msg->getAgentPopulation();
        MASS_base::log(convert.str());
    }
    sendMessage(msg);
    delete msg;
}

void MProcess::sendReturnValues(void *argument, int nPlaces, int return_size) {
    Message *msg = new Message(Message::ACK, argument, nPlaces * return_size);
    sendMessage(msg);
    if (printOutput) MASS_base::log("Sent return value");
    delete msg;
}

void MProcess::sendReturnValues(void *argument, int nAgents, int return_size,
                                int localPopulation) {
    Message *msg = new Message(Message::ACK, argument, nAgents * return_size,
                               localPopulation);
    sendMessage(msg);
    delete msg;
}

void MProcess::sendMessage(Message *msg) {
    int msg_size = 0;
    char *byte_msg = msg->serialize(msg_size);

    write(sd, (void *)&msg_size, sizeof(int));  // send a message size
    write(sd, byte_msg, msg_size);              // send a message body
}

Message *MProcess::receiveMessage() {
    int size = -1;
    int nRead = 0;
    if (read(sd, (void *)&size, sizeof(int)) > 0) {  // receive a message size

        ostringstream convert;
        if (printOutput == true) {
            convert << "receiveMessage: size = " << size << endl;
            MASS_base::log(convert.str());
        }

        char *buf = new char[size];
        for (nRead = 0; (nRead += read(sd, buf + nRead, size - nRead)) < size;)
            ;
        Message *m = new Message();
        m->deserialize(buf, size);
        return m;
    } else {
        if (printOutput == true) MASS_base::log("receiveMessage error");
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    // receive all arguments
    char *cur_dir = argv[1];
    char *hostName = argv[2];
    int myPid = atoi(argv[3]);
    int nProc = atoi(argv[4]);
    int nThr = atoi(argv[5]);
    int port = atoi(argv[6]);

    // set the current working directory to where the master node is running.
    chdir(cur_dir);

    // launch an MProcess at each slave node.
    MProcess process(hostName, myPid, nProc, nThr, port);
    process.start();
}
