
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

#ifndef MESSAGE_H
#define MESSAGE_H
#define VOID_HANDLE -1

#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "RemoteExchangeRequest.h"
#include "AgentMigrationRequest.h"

#include "FileParser.h"
#include "GraphModel.h"



using namespace std;

class Message {

public:

  /**
   * ACTION_TYPE
   * A list of actions assigned to numbers.
   */
  enum ACTION_TYPE
  {
    EMPTY, // 0 
    FINISH, // 1
    ACK, // 2

    PLACES_INITIALIZE, // 3
    PLACES_CALL_ALL_VOID_OBJECT, // 4
    PLACES_CALL_ALL_RETURN_OBJECT, // 5
    PLACES_CALL_SOME_VOID_OBJECT, // 6
    PLACES_CALL_SOME_RETURN_OBJECT, // 7
    PLACES_EXCHANGE_ALL, // 8
    PLACES_EXCHANGE_ALL_REMOTE_REQUEST, // 9
    PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT, // 10
    PLACES_EXCHANGE_BOUNDARY, // 11
    PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST, // 12

    AGENTS_INITIALIZE, // 13
    AGENTS_CALL_ALL_VOID_OBJECT, // 14
    AGENTS_CALL_ALL_RETURN_OBJECT, // 15
    AGENTS_MANAGE_ALL, // 16
    AGENTS_MIGRATION_REMOTE_REQUEST, // 17

    /*---------------------------------------------------------------------*
     *+++++++++++ Elias--> Added for Graph features ++++++++++++++++++++++++
     *----------------------------------------------------------------------*/
    PLACES_INITIALIZE_GRAPH_FROM_FILE, //18
    PLACES_INITIALIZE_GRAPH_FROM_EMPTY, //19
    MAINTENANCE_REINITIALIZE, //20
    MAINTENANCE_GET_PLACES, //21
    MAINTENANCE_GET_PLACES_RESPONSE, //22
    MAINTENANCE_ADD_EDGE, //23
    MAINTENANCE_REMOVE_EDGE, //24
    MAINTENANCE_ADD_PLACE, //25
    MAINTENANCE_REMOVE_PLACE, //26
    GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT, //27
    GET_GRAPH_DISTRIBUTED_MAP,//28
    GET_VERTEXPLACE_FROM_VERTEXNAME //29
  };


  /*----------------------------------------------------------------------------*
   *=============================================================================*/

/**Graph--->PLACES_INITIALIZE_GRAPH_FROM_FILE
* @param action          //type of action this message is performing
* @param size
* @param handle         // unique identifer that designates a group of places.
   *                        //Must be unique over all machines.
* @param classname      // name of user-defined class message is targeting
* @param argument
* @param arg_size
* @param boundary_width
* @param hosts

* @type
* @filename
* @map<string, int>
*/
Message (ACTION_TYPE action, vector<int> *size, int handle, string classname, 
        void *argument,int arg_size, int boundary_width, vector<string> *hosts,
        std::string type, string filename, std::unordered_map<std::string, int> *dist_map)   
              
        :action (action), size (size),
         handle (handle), dest_handle (VOID_HANDLE),
         functionId (0), classname (classname),
         argument (argument), argument_size (arg_size), return_size (0),
         argument_in_heap (false), hosts (hosts), destinations (NULL),
         dimension (0), agent_population (-1), boundary_width (boundary_width),
         exchangeReqList (NULL),
         migrationReqList (NULL),
         vertexId (""),
         neighborId(""),
         weight(0.0),
         distributed_map(dist_map),
         fileType (type),
         filename(filename),
         graphmodel(NULL)  { };

    
/* PLACES_INITIALIZE 
  * @param action          //type of action this message is performing
   * @param size
   * @param handle         // unique identifer that designates a group of places.
   *                        //Must be unique over all machines.
   * @param classname      // name of user-defined class message is targeting
   * @param argument
   * @param arg_size
   * @param boundary_width
   * @param hosts */

  Message (ACTION_TYPE action,
         vector<int> *size, int handle, string classname, void *argument,
         int arg_size, int boundary_width, vector<string> *hosts)

        :action (action), size (size),
         handle (handle), dest_handle (VOID_HANDLE),
         functionId (0), classname (classname),
         argument (argument), argument_size (arg_size), return_size (0),
         argument_in_heap (false), hosts (hosts), destinations (NULL),
         dimension (0), agent_population (-1), boundary_width (boundary_width),
         exchangeReqList (NULL),
         migrationReqList (NULL),
         vertexId (""),
         neighborId(""),
         weight(0.0),
         distributed_map(NULL),
         fileType (""),
         filename(""),
         graphmodel(NULL)  { };

  /**
   * PLACES_CALL_ALL_VOID_OBJECT,
   * PLACES_CALL_ALL_RETURN_OBJECT,
   * AGENTS_CALL_ALL_VOID_OBJECT,
   * AGENTS_CALL_ALL_RETURN_OBJECT
   * @param action
   * @param handle
   * @param functionId
   * @param argument
   * @param arg_size
   * @param ret_size
   */
  Message (ACTION_TYPE action, int handle, int functionId, void *argument, 
          int arg_size, int ret_size)

         :action (action), size (0),
          handle (handle), dest_handle (VOID_HANDLE),
          functionId (functionId), classname (""),
          argument (argument), argument_size (arg_size), return_size (ret_size),
          argument_in_heap (false), hosts (NULL), destinations (NULL),
          dimension (0), agent_population (-1), boundary_width (0),
          exchangeReqList (NULL), migrationReqList (NULL),

         vertexId (""),
         neighborId(""),
         weight(0.0),
         distributed_map(NULL),
         fileType (""),
         filename(""),
         graphmodel(NULL)  { };

  /**
   * PLACES_EXCHANGE_ALL
   * @param action
   * @param handle
   * @param dest_handle
   * @param functionId
   * @param destinations
   * @param dimension
   */
  Message (ACTION_TYPE action,
           int handle, int dest_handle, int functionId,
           vector<int*> *destinations, int dimension)
           
           :action (action), size (0),
           handle (handle), dest_handle (dest_handle),
           functionId (functionId), classname (""),
           argument (NULL), argument_size (0), return_size (0),
           argument_in_heap (false), hosts (NULL), destinations (NULL),
           dimension (dimension), agent_population (-1), boundary_width (0),
           exchangeReqList (NULL), migrationReqList (NULL),

           vertexId (""),
           neighborId(""),
           weight(0.0),
           distributed_map(NULL),
           fileType (""),
           filename(""),
           graphmodel(NULL)  { };

  /**
   * PLACES_EXCHANGE_ALL_REMOTE_REQUEST
   * @param action
   * @param handle
   * @param dest_handle
   * @param functionId
   * @param exchangeReqList
   */
  Message (ACTION_TYPE action,
           int handle, int dest_handle, int functionId,
           vector<RemoteExchangeRequest*> *exchangeReqList) :
  action (action), size (0),
  handle (handle), dest_handle (dest_handle),
  functionId (functionId), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (exchangeReqList), migrationReqList (NULL),

  vertexId (""),
  neighborId(""),
  weight(0.0),
  distributed_map(NULL),
  fileType (""),
  filename(""),
  graphmodel(NULL)  { };

  /**
   * PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT and 
   * PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST
   * @param action
   * @param retVals
   * @param retValsSize
   */
  Message (ACTION_TYPE action, char *retVals, int retValsSize)
   : action (action), size (0),
    handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
    functionId (0), classname (""),
    argument (retVals), argument_size (retValsSize), return_size (0),
    argument_in_heap (false), hosts (NULL), destinations (NULL),
    dimension (0), agent_population (-1), boundary_width (0),
    exchangeReqList (NULL), migrationReqList (NULL),

    vertexId (""),
    neighborId(""),
    weight(0.0),
    distributed_map(NULL),
    fileType(""),
    filename(""),
    graphmodel(NULL)  { };

  /* +++++++++++++++++++++++ Graph Features ++++++++++++++++++++++++++++++++++++++++
   *-------------------------------------------------------------------------------
    *AGENTS_INITIALIZE

    *MAINTENANCE_REMOVE_PLACE: (Action, handle,vertexId)
    *GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT-->(Action, handle neighborKey);
    *MAINTENANCE_ADD_PLACE(action, handle, vertex(classname), argument, arg_size)

   * @param action
   * @param initPopulation
   * @param handle
   * @param placeHandle
   * @param className
   * @param argument
   * @param argument_size
   */
  Message (ACTION_TYPE action, int initPopulation, int handle,
           int placeHandle, string className, void *argument,
           int argument_size) :
  action (action), size (0), handle (handle), 
  dest_handle (placeHandle),
  functionId (0), classname (className),
  argument (argument), argument_size (argument_size), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (initPopulation), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL),

  vertexId (className),
  neighborId(className),
  weight(0.0),
  distributed_map(NULL),
  fileType(""),
  filename(""),
  graphmodel(NULL)  { };

  /**
   * AGENTS_MANAGE_ALL, PLACES_EXCHANGE_BOUNDARY,
   **Graph----> MAINTENANCE_GET_PLACES(handle, 0), MAINTENANCE_REINITIALIZE(action, handle, int dummy)
   * @param action
   * @param handle
   * @param dummy
   */
  Message (ACTION_TYPE action, int handle, int dummy) :
        action (action), size (0),
        handle (handle), dest_handle (handle),
        functionId (0), classname (""), 
        argument (NULL), argument_size (0), return_size (0),
        argument_in_heap (false), hosts (NULL), destinations (NULL),
        dimension (0), agent_population (-1), boundary_width (0),
        exchangeReqList (NULL), migrationReqList (NULL),

        vertexId (""),
        neighborId(""),
        weight(0.0),
        distributed_map(NULL),
        fileType (""),
        filename(""),
        graphmodel(NULL)  { };

  /**
   * AGENTS_MIGRATION_REMOTE_REQUEST
   * @param action
   * @param agentHandle
   * @param placeHandle
   * @param migrationReqList
   */
  Message (ACTION_TYPE action, int agentHandle, int placeHandle,
           vector<AgentMigrationRequest*> *migrationReqList) :
  action (action), size (0),
  handle (agentHandle), dest_handle (placeHandle),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (migrationReqList),

  vertexId (""),
  neighborId(""),
  weight(0.0),
  distributed_map(NULL),
  fileType (""),
  filename(""),
  graphmodel(NULL)  { };

  /**
   * FINISH
   * ACK
   * @param action
   */
  Message (ACTION_TYPE action) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL),

  vertexId (""),
  neighborId(""),
  weight(0.0),
  distributed_map(NULL),
  fileType(""),
  filename(""),
  graphmodel(NULL)  { };

  /**
   * ACK used for PLACES_CALL_ALL_RETURN_OBJECT
   * @param action
   * @param argument
   * @param arg_size
   */
  Message (ACTION_TYPE action, void *argument, int arg_size) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (argument), argument_size (arg_size), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL),

  vertexId (""),
  neighborId(""),
  weight(0.0),
  distributed_map(NULL),
  fileType(""),
  filename(""),
  graphmodel(NULL)  { };


  /**
   * ACK used for AGENTS_CALL_ALL_RETURN_OBJECT
   * @param action
   * @param argument
   * @param arg_size
   * @param localPopulation
   */
  Message (ACTION_TYPE action, void *argument, int arg_size, int localPopulation)
   : action (action), size (NULL),
    handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
    functionId (0), classname (""),
    argument (argument), argument_size (arg_size), return_size (0),
    argument_in_heap (false), hosts (NULL), destinations (NULL),
    dimension (0), agent_population (localPopulation), boundary_width (0),
    exchangeReqList (NULL), migrationReqList (NULL),

    vertexId (""),
    neighborId(""),
    weight(0.0),
    distributed_map(NULL),
    fileType (""),
    filename(""),
    graphmodel(NULL)  { };

  /**
   * ACK used for AGENTS_INITIALIZE and AGENTS_CALL_ALL_VOID_OBJECT
   * @param action
   * @param localPopulation
   */
  Message (ACTION_TYPE action, int localPopulation) :
  action (action), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (localPopulation), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL),

  vertexId (""),
  neighborId(""),
  weight(0.0),
  distributed_map(NULL),
  fileType (""),
  filename(""),
  graphmodel(NULL)  { };

  
  /**
   * EMPTY
   */
  Message () :
  action (EMPTY), size (NULL),
  handle (VOID_HANDLE), dest_handle (VOID_HANDLE),
  functionId (0), classname (""),
  argument (NULL), argument_size (0), return_size (0),
  argument_in_heap (false), hosts (NULL), destinations (NULL),
  dimension (0), agent_population (-1), boundary_width (0),
  exchangeReqList (NULL), migrationReqList (NULL),

  vertexId (""),
  neighborId(""),
  weight(0.0),
  distributed_map(NULL),
  fileType (""),
  filename(""),
  graphmodel(NULL)  { };

  /**Graph----->
    *MAINTENANCE_REMOVE_EDGE(Action, handle,vertexId, neighborId, -0.0)
    *MAINTENANCE_ADD_EDGE(action,handle, vertexId, neighborId, weight)
    @handle 
    @vertexId
    @neighborId
    @weight
    */
  Message(ACTION_TYPE action, int handle, string vertexId, string neighborId, double weight):
  action(action),size(NULL), handle(handle), 
  dest_handle(VOID_HANDLE), functionId (0),
  classname (""),  argument (NULL),  argument_size (0), 
  return_size (0), argument_in_heap (false), 
  hosts (NULL), destinations (NULL), dimension (0), 
  agent_population (-1), boundary_width (0),
  exchangeReqList (NULL),  migrationReqList (NULL),

  vertexId (vertexId),
  neighborId(neighborId),
  weight(weight),
  distributed_map(NULL),
  fileType (""),
  filename(""),
  graphmodel(NULL) { };

 
  /**
   * 
   */
  virtual ~Message (); // delete argument and hosts.

  virtual char *serialize (int &size);
  virtual void deserialize (char *msg, int size);

  /**
   * Get the action
   * @return action
   */
  ACTION_TYPE getAction (){
    return action;
  };

  /**
   * Get the size
   * @return *size
   */
  vector<int> getSize ()
  {
    return *size;
  };

  /**
   * Get the handle
   * @return handle
   */
  int getHandle ()
  {
    return handle;
  };

  /**
   * Get the destination handle
   * @return dest_handle
   */
  int getDestHandle ()
  {
    return dest_handle;
  };

  /**
   * Get the functionId
   * @return functionId
   */
  int getFunctionId ()
  {
    return functionId;
  };

  /**
   * Get the class name
   * @return classname
   */
  string getClassname ()
  {
    return classname;
  };

  /**
   * Check if argument is valid
   * @return (argument != NULL)
   */
  bool isArgumentValid ()
  {
    return ( argument != NULL);
  };

  /**
   * Get the argument via memcpy
   */
  void getArgument (void *arg)
  {
    memcpy (arg, argument, argument_size);
  };

  /**
   * Get the argument pointer
   * @return argument
   */
  void *getArgumentPointer ()
  {
    return argument;
  };

  /**
   * Get the argument size
   * @return argument_size
   */
  int getArgumentSize ()
  {
    return argument_size;
  };

  /**
   * Get the return size
   * @return return_size
   */
  int getReturnSize ()
  {
    return return_size;
  };

  /**
   * Get the Boundary Width
   * @return boundary_width
   */
  int getBoundaryWidth ()
  {
    return boundary_width;
  };

  /**
   * Get the Agent Populations
   * @return agent_population
   */
  int getAgentPopulation ()
  {
    return agent_population;
  };

  /**
   * Get the hosts
   * @return *hosts
   */
  vector<string> getHosts ()
  {
    return *hosts;
  };

  /**
   * Get the destinations
   * @return destinations
   */
  vector<int*> *getDestinations ()
  {
    return destinations;
  };

  /**
   * Get the Remote Exchange Request List
   * @return exchangeReqList
   */
  vector<RemoteExchangeRequest*> *getExchangeReqList ()
  {
    return exchangeReqList;
  };

  /**
   * Get the Agent Migration Request List
   * @return migrationReqList
   */
  vector<AgentMigrationRequest*> *getMigrationReqList ()
  {
    return migrationReqList;
  };

/*---------------------------------------------------------------------------
 *+++++++++++++++++++++++++Added for graph features++++++++++++++++++++++++++++
  *---------------------------------------------------------------------------*/
  //name of the vertex
  string getVertexId() {
     return vertexId;
  };

  //name of the veighbor vertix
  string getNeighborId(){
    return neighborId;

  };
  //weight of the edge
  double getweight(){
    return weight;
};

std::unordered_map <string, int>* getDistributed_map(){
  return distributed_map;
};

string getFileType(){
  return fileType;
};

string getFilename(){
  return filename;
};

GraphModel* getGraphModel(){
  return graphmodel;
};
//------------------------------------------------------------------------------------------
protected:
  
  ACTION_TYPE action;
  vector<int> *size;
  int handle;
  int dest_handle;
  int functionId;
  string classname; // classname.so must be located in CWD.
  void *argument;
  int argument_size;
  int return_size;
  bool argument_in_heap;
  vector<string> *hosts; // all hosts participated in computation
  vector<int*> *destinations; // all destinations of exchangeAll
  int dimension;
  int agent_population;
  int boundary_width;
  vector<RemoteExchangeRequest*> *exchangeReqList;
  vector<AgentMigrationRequest*> *migrationReqList;

  //added for graph
  string vertexId;
  string neighborId;
  double weight;
  std::unordered_map<string, int> *distributed_map;
  string fileType;
  string filename;
  GraphModel *graphmodel;
};

#endif
