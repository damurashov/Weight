
#include "GraphPlaces.h"

//stl includes
#include<iostream>
#include <memory> //shared_ptr
#include <algorithm>    // std::remove
#include <iterator>

//MASS lib includes
#include "Message.h"
#include "GraphModel.h"
#include "MASS.h"
#include "Place.h"




// Used to toggle comments from GraphPlaces
#ifndef LOGGING
const bool printOutput = true;
#else
const bool printOutput = false;
#endif

/**
 * Creates a GraphPlaces Object that serves as a container Object for the
 * neighborhood (collection) of each individual VertexPlace in a simulation, providing
 * methods/functionality that corresponds with the entire simulation space.
 *
 * Instantiates a shared array with "size[]" from the "className" class by
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 
 *Arguments:
 * @param handle          a unique identifer that designates a group of Place
 *                        Objects as all belonging to the same simulation
 *                        (Places). Must be unique over all machines.
 * @param className       name of user-created Places class to load
 * @param boundary_width  the boundasry witdth between palces
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 * @param dimension       how many dimensions this simulation encompasses
 * @param filename		  the absolute filename that the vertices will read from
 * @param type 			  the file enum(FILE_TYPE_ENUMS), example TXT, HIPPIE, MATSim etc.                       
 */
GraphPlaces::GraphPlaces(int handle, std::string className,int boundary_width,int dimension, string filename,
                     	FILE_TYPE_ENUMS type, void* argument, int arg_size)
						:Places(handle, className, boundary_width, dimension,filename,type,argument,arg_size ){
							
	this->handle = handle;
	this->className = className;
	this->filename = filename;
    this->type = type;

	if (printOutput) {
		ostringstream convert;
        convert << "GraphPlaces --> Handle = " << handle << " className = "
        << className << " filename = " << filename << " dimension " << dimension
        <<" boundary_width " << boundary_width << " arg_size " << arg_size << " argument "
        << (char *)argument << endl;
        MASS_base::log(convert.str());
    }
    init_master_base(argument, arg_size, boundary_width);
       
}

/**
 * Creates a GraphPlaces Object that serves as a container Object for the
 * neighborhood (collection) of each individual VertexPlace in a simulation, providing
 * methods/functionality that corresponds with the entire simulation space.
 *
 * Instantiates a shared array with "size[]" from the "className" class by
 * passing an argument to the "className" constructor. This array is
 * associated with a user-given handle that must be unique over
 * machines.
 
 *Arguments:
 * @param handle          a unique identifer that designates a group of Place
 *                        Objects as all belonging to the same simulation
 *                        (Places). Must be unique over all machines.
 * @param className       name of user-created Places class to load
 * @param boundary_width  the boundasry witdth between palces
 * @param argument        argument to pass into each Place constructor
 * @param arg_size   total size of the argument
 * @param dimension       how many dimensions this simulation encompasses   
 * @param nVertices       number of vertices that the graph should have                       
 */
GraphPlaces::GraphPlaces(int handle, string className, int boundary_width,int dimension,void* argument,
				 		 int arg_size,int nVertices)
						:Places(handle, className, boundary_width, dimension, argument, arg_size, nVertices){
	if (printOutput) {
		ostringstream convert;
        convert << "GraphPlaces --> Handle = " << handle << " className = "
        << className << " dimension " << dimension
        <<" boundary_width " << boundary_width << " arg_size " << arg_size << " argument "
        << (char *)argument << " number of vertices " << nVertices << endl;
        MASS_base::log(convert.str());
    }
    init_master_base(argument, arg_size, boundary_width);
}

//clear out all VertexPlace pointers
GraphPlaces::~GraphPlaces(){
	
}



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *__________________________ Graph maintainance feature implementatiosn __________________________________
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 *This method returns array of VertexPlace objects residing on the
 * master node.The list includes both dinamically added Places and Places
 *constructed by reading vertices from the filename.               
 */
 GraphModel* GraphPlaces::getGraphOnThisNode(){
 	GraphModel *graphmodel = new GraphModel();
 	if(places_size > 0){
 		//map<int, DllClass *> dllMap;// a collection of DllClasses/place/VertexPlace
 		auto dllclass = MASS_base::dllMap.find(handle);
 		DllClass * dclass = dllclass->second;
 		for(int i = 0;  i < places_size; i++){
 			VertexPlace *vertexplace = (VertexPlace*)dclass->places[i];

 			string vertexname = vertexplace->getVertexName();
 			std::vector<string> neigbors = vertexplace->getNeighbors();
 			std::vector<double> coonectionWeight = vertexplace->getWeights();
 			graphmodel->addVertexModel(vertexname ,neigbors, coonectionWeight);
 			
 		} 		
 	}
 	for(int i = 0; i < (int)placesVector.size(); i++){
 		std::vector<VertexPlace*> outerVector = placesVector.at(i);
 		for(int j = 0; j < (int) outerVector.size(); j++){
 			VertexPlace * place = outerVector.at(j);

 			string vertexname = place->getVertexName();
 			std::vector<string> neigbors = place->getNeighbors();
 			std::vector<double> coonectionWeight = place->getWeights();
 			graphmodel->addVertexModel(vertexname ,neigbors, coonectionWeight);
 		}
 	}
 	return graphmodel;

 }

/**
 *This method returns an array of VertexPlace objects residing on the
 * entire simulation space.The list includes both dinamically added Places and Places
 *constructed by reading vertices from the filename.

 *Calling this method is expennsive since all the simulation space will converge to the 
 *master node's memory and may overwhelm the master node.             

 */
 GraphModel* GraphPlaces::getAllGraphOnTheCluster(){
 	//get the local vertices first
 	GraphModel *graphmodel = getGraphOnThisNode();
 	GraphModel remoteGraphModel =  getRemoteGraphs();

 	merge(*graphmodel, remoteGraphModel);
 	return graphmodel;
 }


/**
 *Given a graph, this method erases the existing graph data and resets all the 
 *values to the new graph.

 *This functionality is implemented to use in the Cytoscape use case. When the Cytoscape
 *server sends the graph back, resetting the graph to the incoming graph rather than reconstructing it
 *seems performant.

 */ 
void GraphPlaces::setGraph(GraphModel &newGraph){
	 clearGraphOnTheCluster();
	 const std::vector<VertexModel*> &vertex  = newGraph.getGraphVertices();
	 for(int i = 0; i < (int)vertex.size(); i++){//VertexModel model:vertex){
	 	VertexModel* model = vertex.at(i);
	 	string vertexname = model->getVertexPlaceName();
	 	addVertex(vertexname); /*TODO better to send to the remote node a list of VertexPlace
	 							* objects as compared to sending one by one. addvertex method sends them one by one*/ 
	 }

	 for(int i = 0; i < (int)vertex.size(); i++){
	 	/*****Structure of VertexModel object*******
	 		string vertexPlaceName;
			std::vector<string> neighbors;
			std::vector<double> weights;*/
	 	VertexModel* model = vertex.at(i);
	 	std::string vertexname = model->getVertexPlaceName();;
	 	std::vector<string> neighbors = model->getVertexPlaceNeighbors();
	 	std::vector<double> weight = model->getVertexPlaceWeights();
	 	for(int j = 0; j < (int)neighbors.size(); j++){

	 		string nameOfVertex = neighbors.at(j);
	 		double weightOfTheConnection = weight.at(j);
	 		addEdge(vertexname, nameOfVertex, weightOfTheConnection);
	 	}
	 }	 
}

/**
 *This method merges two Graph model objects.
 *After merging, the GraphModel on the left will contain the both data, while the GraphModel on the right
 *remains the same

 */
void GraphPlaces::merge(GraphModel &source, GraphModel &remoteGraphs){
	//std::vector<VertexModel*> vmodel = source.getGraphVertices();
	const std::vector<VertexModel*> &remoteVmodel = remoteGraphs.getGraphVertices();;

	for(int i = 0; i < (int)remoteVmodel.size(); i++){
		source.getGraphVertices().push_back(remoteVmodel.at(i));
	}
}


/**
 *This method collects GrpahModel objects from remote nodes and
 *returns the agregated GrpahModel object
 *Note: the local node GraphModel object is not included in the agregate

 */
GraphModel GraphPlaces::getRemoteGraphs(){
	 GraphModel *graphmodel = new GraphModel();
	
	/*@action: MAINTENANCE_GET_PLACES
	 *@param: handle
	 *@param: 0 dummy
	 */
	Message *message = new Message(Message::MAINTENANCE_GET_PLACES, getHandle(), 0);

	//send message to all remote nodes
	for(int i = 0; i < int(MASS::mNodes.size()); i++){
		 MASS::mNodes[i]->sendMessage(message);
	
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--> getRemoteGraphs()--->ACTION_TYPE:-> MAINTENANCE_GET_PLACES" 
			<<" message sent to node: " << MASS::mNodes[i]->getHostName() << endl;
			MASS_base::log(convert.str()); 
		}
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--> in getRemoteGraphs method " 
			<<" wating for ack from " <<  MASS::mNodes[i]->getHostName() << endl;
			MASS_base::log(convert.str()); 
		}

		//waite for the resposne
		 message =  MASS::mNodes[i]->receiveMessage();

		 //check if the response comes with the correct action type
		 if(message->getAction() != Message::MAINTENANCE_GET_PLACES_RESPONSE){
		 	ostringstream convert;
		 	convert << "Incorrect type of response from node " << MASS::mNodes[i]->getHostName() << endl;
		 	MASS_base::log(convert.str());
		 }

		 if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--> in getRemoteGraphs method " 
			<<" GraphModel object received from " <<  MASS::mNodes[i]->getHostName() << endl;
			MASS_base::log(convert.str()); 
		}

		//collect the response from remote nodes
		GraphModel remoteGraphModel = *(message->getGraphModel());

		//merge the response
		merge(*graphmodel, remoteGraphModel);
	}
	delete message;
	return *graphmodel;
}

/**
 *This method adds an edge between two vertices

 *@param vertexId 		name of a vertex
 *@param neighborId 	neighbor vertex
 *@param weight 		weight of the connection
 *@return 				true if the edge formed on this local node(master mode),
  false if the edge formed on a remote node. Also return false if either the vertex or the 
 *neighbor look up returns NULL from the MASS_base::distributed_map.

 */
bool GraphPlaces::addEdge(string vertexId, string neighborId, double weight){
	bool vertex = VertexFoundInDistributedMap(vertexId);
	bool neighbor = VertexFoundInDistributedMap(neighborId);

	//if neither the vertex nor the neighbor isn't in the map, 
	//addEdge request will denied.
	
	if(!vertex || !neighbor){
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces::addEdge()--> addEdge action denied. "
			<<"Either vertex or the neighbor isn't in the MASS_base::distributed_map" << endl;
			MASS_base::log(convert.str());
		}
		return false;
	}
	std::unordered_map<string, int>* dist_map = getThisDistributedMap(handle);
	auto it  = dist_map->find(vertexId);
	if(it == dist_map->end()){
	 	if(printOutput){
	 		ostringstream convert;
	 		convert << "the vertex is not in the MASS_base::distributed_map. "
	 		<<" No edge formed between the vertex" << vertexId << " and its neighbor " << neighborId << endl;
	 		MASS_base::log(convert.str());
	 	}
	 	return false;
	 }

	int globalIndex = it->second;
	int nodePid = getNodeIdFromGlobalLinearIndex(globalIndex);

	//add edge on this node
	if( MASS_base::myPid == nodePid){
		if(printOutput){
			ostringstream convert;
			convert << "GraphPaces-->AddEdge()--> called addEdgeLocally() to form edge locally." << endl;
			MASS_base::log(convert.str());
		}
		return addEdgeLocally(vertexId, neighborId, weight);
	}

	/*ACTION_TYPE action, vector<int> *size, int handle, string classname, 
        void *argument,int arg_size, int boundary_width, vector<string> *hosts,
         std::string type, string filename, std::map<string, int> *myMap*/
	string myWeight = std::to_string(weight);
	std::vector<string>* neighborInformation = new std::vector<string> ();
	neighborInformation ->push_back(vertexId);
	neighborInformation ->push_back(neighborId);
	neighborInformation ->push_back(myWeight);

	Message* message = new Message(Message::MAINTENANCE_ADD_EDGE, NULL, getHandle(), "", NULL, 0,
					   0, neighborInformation);//, "", ""); 

	std::vector<MNode*> vecHost = MASS::mNodes;
	bool messageSent =  false;
	for(int i = 0 ; i < (int)vecHost.size(); i++){
		MNode* node = vecHost.at(i);
		if(node->getPid() == nodePid){
			node->sendMessage(message);
			messageSent = true;

			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces-->addEdge() -->actio type = MAINTENANCE_ADD_EDGE "
				<<" message sent to host " << node->getHostName() << endl;
				MASS_base::log(convert.str());
			}
			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces-->addEdge() waiting for ack from " << node->getHostName() << endl;
				MASS_base::log(convert.str());
				
			}
			message = node->receiveMessage();
			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces-->addEdge()-->ack receiveed from " << node->getHostName() << endl;
				MASS_base::log(convert.str());

			}
			
			delete message;
			delete neighborInformation;
			return false;
		}
	}
	if(messageSent == false){
		ostringstream convert;
		convert << "GraphPlaces-->addEdge()--> host for MAINTENANCE_ADD_EDGE message couldn't be found." 
		<< endl;
		MASS_base::log(convert.str());
		
		delete message;
		delete neighborInformation;
		return false;
	}
	delete message;
	delete neighborInformation;
	return false;
}

/**
 * This method creates edges between two vertices

 *create an edge between two vertex with the given weight on the local machine.

 *@pama vertexId 		The string representation of the vertex.
 *@pama neighborId 		the name of the neighbor
 *@pama weight 			the weight of the connection between the two vertices
 *@return 		 		true if edge formed, false otherwise

 */
bool GraphPlaces::addEdgeLocally(string vertexId, string neighborId, double weight){
	
	auto it  = places_base_distributed_map->find(vertexId);
	int globalIndex = it->second;
	int total = 1;
	for(int i = 0; i < dimension; i++){
		total *= Places_base::size[i];
	}

	int strip = total / MASS_base::systemSize;
	int placesIndex = globalIndex / total - 1;

	//determine the position to insert the neighbor and the connection 
	if(placesIndex >= 0 && (int)placesVector.size() >= 0 && (int)placesVector.size() < placesIndex){
		int localPlaceIndex =globalIndex % strip;
		VertexPlace *vplace = placesVector.at(placesIndex).at(localPlaceIndex);
		vplace->addNeighbor(neighborId, weight);

		if(printOutput){
			ostringstream convert;
			convert << "GraphPaces-->addEdgeLocally() ---> Edge formed between " << vertexId << " and "
			<< neighborId << " with weight = " << weight << endl;
			MASS_base::log(convert.str());
		}
		return true;
	}

	if(printOutput){
		ostringstream convert;
		convert << "GraphPaces-->addEdgeLocally() ---> Edge cound not be formed between " << vertexId <<" and "
		<< neighborId << ".\nEither the edge already exists or you are trying to add a value out of bound." << endl;
		MASS_base::log(convert.str());
	}
	return false;
}
/**
 * This method removes an edge between two verties

 *given two vertices, this methods finds the location of the vertices
 *and removes the edge bwtween them. If the vertices live on the remote machies
 *the method sends a broadcasting message to all nodes to remove the edge between 
 *these vertices.

 *@param vertexId 			the name of the vertex as a string
 *@param neighborId 		the nighbor vertices for the first argument.
 *@return 					true if the vertices found and removed from the local machine.
 otherwise sends a broadcasting message to all nodes and returns false to the calling computing node.

*/
bool GraphPlaces::removeEdge(string vertexId, string neighborId){

	bool vertex = VertexFoundInDistributedMap(vertexId);
	bool neighbor = VertexFoundInDistributedMap(neighborId);

	if(!vertex || !neighbor){
		if(printOutput){
			ostringstream convert;
			convert <<"GraphPlaces -->removeEdge() -->"
			<< "either the vertex and/or the neighbor vertex couldn't be found in the simulation space."
			<<endl;
			MASS_base::log(convert.str());
		}
		return false;
	}
	
	
	unordered_map<string, int>* dist_map = getThisDistributedMap(handle);
	auto const &it = dist_map->find(vertexId);

	if(it == places_base_distributed_map->end()){
	 	if(printOutput){
	 		ostringstream convert;
	 		convert << " GraphPlaces-->removeEdge()---> The vertex is not in the MASS_base::distributed_map. "
	 		<<" No edge removed between the vertex" << vertexId << " and its neighbor " << neighborId << endl;
	 		MASS_base::log(convert.str());
	 	}
	 	return false;
	 }
	
	int globalIndex = it->second;
	int nodePid = getNodeIdFromGlobalLinearIndex(globalIndex);

	//add edge on this node
	if( MASS_base::myPid == nodePid){
		if(printOutput){
			ostringstream convert;
			convert << "GraphPaces-->AddEdge()--> called removeEdgeLocally() to atempt to remove edge locally. " <<endl;
			MASS_base::log(convert.str());
		}
		return removeEdgeLocally(vertexId, neighborId);
	}

	std::vector<string> *neighborInformation = new std::vector<string>();
	neighborInformation->push_back(vertexId);
	neighborInformation->push_back(neighborId);
	neighborInformation->push_back("0.0");

	/*ACTION_TYPE action, vector<int> *size, int handle, string classname, 
        void *argument,int arg_size, int boundary_width, vector<string> *hosts,
         std::string type, string filename, std::map<string, int> *myMap*/
	Message* message = new Message(Message::MAINTENANCE_REMOVE_EDGE, NULL, getHandle(),
						"", NULL, 0, 0, neighborInformation);//, "", "");   

	std::vector<MNode*> vecHost = MASS::mNodes;
	bool messageSent =  false;
	for(int i = 0; i < (int)vecHost.size(); i++){
		MNode* node = vecHost.at(i);
		if(node->getPid() == nodePid){
			
			node->sendMessage(message);
			messageSent = true;

			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces-->removeEdge() -->actio type = MAINTENANCE_REMOVE_EDGE "
				<<" message sent to host " << node->getHostName() << endl;
				MASS_base::log(convert.str());
			}
			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces-->removeEdge() waiting for ack from " << node->getHostName() << endl;
				MASS_base::log(convert.str());
				
			}
			
			message = node->receiveMessage();
			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces-->removeEdge()-->ack receiveed from " << node->getHostName() << endl;
				MASS_base::log(convert.str());
			}
			delete neighborInformation;
			delete message;
			//if(m != NULL)delete m;
			return false;//false returned because edge is formed on the remote node
		}
	}

	if(messageSent == false){
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces-->removeEdge()--> host couldn't be found. Edge not removed. "
			<< endl;
			MASS_base::log(convert.str());
		}
		delete neighborInformation;
		delete message;
		//if(m != NULL)delete m;
		return false;
	}
	delete neighborInformation;
	delete message;
	//if(m != NULL)delete m;
   	return false;

}

/**
 *this method removes an edge between two neighbors on a local machine.

 *The method finds the two vertices and remves the edge between them if exists and returns
 *true, returns false othewise.
 *@param vertexId 			the principal vertex
 *@param neighborId 		the neighbor vertex
 *@return  					true for successful removal of edge, false otheriwse.

 */
bool GraphPlaces::removeEdgeLocally(string vertexId, string neighborId){
	unordered_map<string, int> *dist_map = getThisDistributedMap(handle);
	auto it = dist_map->find(vertexId);
	int globalIndex = it->second;
	int total = 1;

	for(int i = 0; i < Places_base::dimension; i++){
		total = Places_base::size[i];
	}

	int strip = total / MASS_base::systemSize;

	int placesIndex = globalIndex / total - 1;

	if(placesIndex >= 0 && (int)placesVector.size() >= 0 && (int)placesVector.size() > placesIndex){
		 int localPlaceIndex = globalIndex % strip;

		 VertexPlace *place = placesVector.at(placesIndex).at(localPlaceIndex);
		 place->removeNeighbor(neighborId);

		 if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces-->removeEdgeLocally()--> edge  between " 
			<< vertexId << " and " << neighborId << " removed "<< endl;
			MASS_base::log(convert.str());
		}
		return true;
	}
	else{
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces-->removeEdgeLocally --> dinamic removal is only supported on newly added "
			<< " vertices: The one added after reading from file.\n "	<< vertexId << " and " << neighborId 
			<<" are static portion of the graph. "<< endl;
			MASS_base::log(convert.str());
		}
		return false;
	}
}

/**
 
 *this method adds a vertex on the simulation space.

 *given a vertex name(id), if the vertex is not already in the simulation space,
 *this method adds the vertex to the simulation space. If already in simulation space,
 *the method returns -1 without adding the vertex. vertices addition is at-most-once-semantics
 *@param vertexId 		the name of the vertex to be added.
 *@return 				the global id of the vertex if added successfuly, -1 otherwise.

 */
int GraphPlaces::addVertex(string vertexId){
		if(VertexFoundInDistributedMap(vertexId)){
				if(printOutput){
					ostringstream convert;
					convert << "Wrong action: Vertex " << vertexId 
					<< " is already in the simulation space." << endl;
					MASS_base::log(convert.str());
				}
				return -1;
		}
		
		int nodePid  = getNodeIdFromGlobalLinearIndex(nextPlaceIndex);
		string hostname = getHostNameFromPid(nodePid);
		int returnVal = addVertexPlace(hostname, vertexId, NULL, 0);
		return returnVal;
}


/**
 *this method adds a vertex on the simulation space.

 *given a vertex name(id), if the vertex is not already in the simulation space,
 *this method adds the vertex to the simulation space. If already in simulation space,
 *the method returns -1 without adding the vertex. vertices addition is at-most-once-semantics
 *@param vertexId 		unique name of the vertex
 *@param argument 		argument for the place constructor
 *@param agr_size: 		size of the argument 
 *@return 				the global id of the vertex if added successfuly, -1 otherwise

 */
int GraphPlaces::addVertex(string vertexId, void *argument, int arg_size){
	if(VertexFoundInDistributedMap(vertexId)){
		if(printOutput){
			ostringstream convert;
			convert << "Wrong action: Vertex " << vertexId 
			<< " is already in the simulation space." << endl;
			MASS_base::log(convert.str());
		}
		return -1;//the vertex is already in the cluster, so no need to add it
	}
    
    int nodePid = getNodeIdFromGlobalLinearIndex(nextPlaceIndex);
    string hostname = getHostNameFromPid(nodePid);
    int retVal = addVertexPlace(hostname, vertexId, argument, arg_size);    
    return retVal;
}
	
/**
 *a private method to add the vertices on the given hostname.

 *This method addes a Place object to the simulation space.
 *@param host 			the name of the host computer to add the vertex
 *@param vertexId 		the name of the vertex
 *@param argument 		the argument to create the vertexPlace (Place) object
 *@param int agr_size 	the size of the argument 

*/
int GraphPlaces::addVertexPlace(string host, string vertexId, void* argument, int arg_size){

		//string myHostname = MASS_base::getMyHostName();
		if(host == ""){
			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces--> addVertexPlace method  called addPlaceLocally method to add "
				<< vertexId <<" locally." << endl;
				MASS_base::log(convert.str());
			}
			return addPlaceLocally(vertexId, argument, arg_size);
		}

		std::vector<string> listOfHosts;
		string destHost = "";
		//get list of hostnames in the cluster
		getListOfHostsAsAstring(listOfHosts);
		for(int i = 0; i < (int)listOfHosts.size(); i++){
			string destination = listOfHosts.at(i);
			if(host == destination){
				destHost = host;
				break;
			}
		}

		if(destHost == ""){
			if(printOutput){
				ostringstream convert;
				convert << "GraphPlaces--> addVertex()--> unable to add vertex. Host not found " << endl;
				MASS_base::log(convert.str());
			}
			return -1;
		}

		/*@param action-->(action, int initPopulation, int handle, int placeHandle, string className, void *argument,int arg_size
	   	 * @param initPopulation
	   	 * @param handle
	     * @param placeHandle
	     * @param className
	     * @param argument
	     * @param argument_size*/	
		Message* message = new Message(Message::MAINTENANCE_ADD_PLACE, 0, getHandle(), 0, vertexId, argument, arg_size);

		//get the rank of the destination host and send the message
		int pid = getRankFromGlobalLinearIndex(nextPlaceIndex);
		MASS::mNodes[pid]->sendMessage(message);
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--->Message::MAINTENANCE_ADD_PLACE message sent to node "
			<< " with pid " << pid << " hostname " << MASS::mNodes[pid]->getHostName() << endl;
			MASS_base::log(convert.str());
		}
		
		//receive ack message
		message = MASS::mNodes[pid]->receiveMessage();
		int agent_population = message->getAgentPopulation();
		
		delete message;
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--->Message::MAINTENANCE_ADD_PLACE message ack received from "
			<< " pid " << pid << " hostname " << MASS::mNodes[pid]->getHostName() 
			<< "message deleted " << endl;

			MASS_base::log(convert.str());
		}
		return agent_population;
}

/** 
 *This method adds a vertex to the local machine.

 *Given vertexId and argument along with size of argument to construct VertexPlace object,
 *this method creates the object and inserts it to the simulation space
 *@param vertexId 			name of the vertex to be inserted
 *@param argument 			to construct VertexPlace obect
 *@arg_size 				size of the argument
 *@return					positive number if inserted, -1 otherwise

 */
int GraphPlaces::addPlaceLocally(string vertexId, void* argument, int arg_size){

	int total = 1;
	
	for(int i = 0; i < dimension; i++){
		total *= Places_base::size[i];
	}
	
	//[0] ->nextPlaceIndex/total, [1] -> nextPlaceIndex % total
	int* placesIndex = getPlacesIndex();
	 
	int stripSize = total / MASS_base::systemSize;
	int lowerBound = stripSize * MASS_base::myPid * placesIndex[0];
	int upperBound = lowerBound + stripSize;

	if((int)placesVector.size() == 0 || (int)placesVector.size() < placesIndex[0]){
		std::vector<VertexPlace*>  v (stripSize);
		placesVector.push_back(v);
	}

	else if(placesIndex[1] < lowerBound || placesIndex[1] >= upperBound){
		ostringstream convert;		
		convert << " GreaphPlaces--> AddPlaceLocally() --> unable to add vertex. out of bound error raised. " << endl;		
		MASS_base::log(convert.str());
		return -1;
	}

	// load the construtor and destructor
    DllClass* dllclass = new DllClass(this->className);

	VertexPlace * vertexPlace = (VertexPlace *)(dllclass->instantiate(argument));

	vertexPlace->setVertexName(vertexId);
	int globalIndex = total + stripSize * MASS_base::myPid + nextPlaceIndex;	
	vertexPlace->setGlobalId(globalIndex);

	
	 std::vector<VertexPlace*> &v  = placesVector.at(placesIndex[0]);
	  v.insert(v.begin() + placesIndex[1], vertexPlace);
	 places_base_distributed_map->insert({vertexId, globalIndex});
	 nextPlaceIndex++;

	 if(printOutput){
	 	ostringstream convert;
	 	convert << "GraphPlaces--> addPlaceLocally()-->Place object inserted into the simualtion space "
	 	<<" with globalIndex of " << globalIndex << endl;
	 	MASS_base::log(convert.str());
	 }
	 return globalIndex;
}


/**
 *this method sends a remove vertex message to all nodes and calls
 * the removeVertexLocally() method to remove the vertex on the local node
 @param: unique name of the vertex
 @true if found and removed, false otherwise

 */
bool GraphPlaces::removeVertex(string vertexId){
	if(!VertexFoundInDistributedMap(vertexId)){
		if(printOutput){
			ostringstream convert;
			convert << "vertex required to be removed doesn't exist on the MASS::distributed_map." << endl;
			MASS_base::log(convert.str());
		}
		return false;
	}
	
	/*action, int initPopulation, int handle,placeHandle, string className, void *argument,int arg_size
	 *@param action
  	 *@param initPopulation
  	 *@param handle
  	 *@param placeHandle
   	 *@param className
   	 *@param argument
   	 *@param argument_size*/

	//send a remove the neighbor vertix from the space message to all nodes.
    Message *message = new Message(Message::MAINTENANCE_REMOVE_PLACE, 0, getHandle(),0, vertexId, NULL, 0);
    sendMessageToAllWorkers(message, "Message::MAINTENANCE_REMOVE_PLACE");
    delete message;
    return removeVertexLocally(vertexId);	
}

/**
 *this method removes the given vertex from the local machine

 *if the given vertex found on the local machine, this method removes the vertex
 *and returns true, false otherwise.
 *@param vertexId 			 the unique name of the vertex
 *@return					 returns true if the vertex found removed, false otherwise

 */
bool GraphPlaces::removeVertexLocally(string vertexId){
	auto it = places_base_distributed_map->find(vertexId);
	if(it == places_base_distributed_map->end()){
		if(printOutput){
			ostringstream convert;
			convert << "vertex required to be removed doesn't exist on the MASS::distributed_map." << endl;
			MASS_base::log(convert.str());
		}
		return false;
	}

	//get the global index 
	int globalIndex = it->second;
	
	VertexPlace *vrtxplace = NULL;
	// Remove this vertex and its neighbor from the simulation space on this node
    for (int i = 0; i < (int)placesVector.size(); i++){ //std::vector<VertexPlace*> &places :placesVector) 
    	vector<VertexPlace*> &places = placesVector.at(i);
        for (int j = 0; j < (int)places.size(); j++) {
        	VertexPlace *singlePlace  = NULL;
        	singlePlace = places.at(j);

            if (singlePlace != NULL && singlePlace->getGlobalIndex() == globalIndex) {
                vrtxplace = singlePlace;
                //singlePlace->clearAllNeighborsAndWeight();
            }
            if(singlePlace)singlePlace->removeNeighbor(vertexId);
        }         
        //memory leak proof removal of VertexPlace pointers.
        if (vrtxplace != NULL) {
        	for(int k = 0 ; k < (int)places.size(); k++){

        		if(places.at(k)->getVertexName() == vrtxplace->getVertexName() ||
        			places.at(k)->getGlobalIndex() == vrtxplace->getGlobalIndex()){

        			if(places.at(k) != nullptr){
        				delete places.at(k);
        				places.at(k) = nullptr;
        			}
        			
        		}
        	}
        	auto iteratorBegin = std::remove(places.begin(), places.end(), nullptr);
        	places.erase(iteratorBegin, places.end());
        }
    }
   
    return true;
}

/**
 *this method calls the callMethod of all Place objects
 
 *Given the unique id of the function to be called
 *this function calls the given method from all place objects in the simulation sapce.
 *The vertual method of the Place class 'callMethod' should be impelemnted for this
 *method to behave as expected.

 */
void GraphPlaces::callPlaceMethod(int functionId, void* argument){
	for(int i  = 0; i < (int)placesVector.size(); i++){//
		vector<VertexPlace*> vectorOfPlaces = placesVector.at(i);
		for(int j = 0 ; j < (int) vectorOfPlaces.size(); j++){
			VertexPlace* vectorPlace = vectorOfPlaces.at(j);
			vectorPlace->callMethod(functionId, argument);			
		}

	}
	
}

/**
 *This method calls the Place objects method indicated on the functionId.

 *This method calls the Place objects method indicated on the functionId and puts the 
 * the returned values in the 'returns' vector .
 *@param functionId 		unique id that identifies the function to be called
 *@param returns 			a vector of void pointers which holds the return values from the function.
 *@param arguments 			a vector of void pointer each element to be passed an an arguments for the function. 
 *@return 					void 

 */
void GraphPlaces::callAllWithReturns(int functionId, vector<void*> &returns,vector<void*> &arguments){

	int bIndex = 1;
	for(int i = 0; i < dimension; i++){
		 bIndex *= size[i];
	}

	for(int vIndex = 0; vIndex < (int)placesVector.size(); vIndex++){
		std::vector<VertexPlace*> places = placesVector.at(vIndex);

		for (int pIndex = 0; pIndex < (int)places.size(); pIndex++) {
                int gIndex = bIndex + bIndex * vIndex + pIndex;

                if ((int)arguments.size() == 0 || gIndex > (int)arguments.size())
                    returns.push_back( places.at(pIndex)->callMethod(functionId, NULL));
                else
                    returns.push_back(places.at(pIndex)->callMethod(functionId, arguments.at(gIndex)));
            }
        }
 }


/**
 * This method helps the Place object to exchange information.

 *Given the function's unique Id, this method helps to exchange information 
 *between Place objects accross the cluster on the simulation space.

 *@ param currentFunctionId			a unique Id for the function
 *@param  handle 					a unique Id of the simulation space.
 *@return 							void

*/
void GraphPlaces::exchangeAll(int currentFunctionId, int handle){
	
    //int myRank = MASS_base::myPid;

    for(int i  = 0; i < (int)placesVector.size(); i++){
    	vector<VertexPlace*> places = placesVector.at(i);
    	for(int j = 0; j < (int)places.size(); j++){
    		VertexPlace* place = places.at(j);
			std::vector<string> neighbors = place->getNeighbors();//

			for(int k = 0; k < (int)neighbors.size(); k++){
				string neighborKey = neighbors.at(k);

			 std::unordered_map<string, int> *dist_map = getThisDistributedMap(handle);	
			  auto it = dist_map->find(neighborKey);
			  int neighborGlobalLinearIndex = it->second;
			  int owner = getNodeIdFromGlobalLinearIndex(neighborGlobalLinearIndex);

			  void* result;
			  if(owner == MASS_base::myPid){
			  	if(printOutput){
			  		ostringstream convert;
			  		convert << "GraphPlaces: \"ExchangeAll\" method --> neighbor found on the mater node. " << endl;
			  		MASS_base::log(convert.str());
			  	}

			  	VertexPlace* neighborPlace = this->getVertexPlaceFromGlobalLinearIndex(neighborGlobalLinearIndex);
			  	result = neighborPlace->callMethod(currentFunctionId, NULL);
			  }
			  else {
                    
			   /*(action, int initPopulation, int handle,int placeHandle, string className, void *argument,arg_size)
			   	*@param action ***needed****
   				* @param initPopulation-->placeholder
   				* @param handle ***needed***
   				* @param placeHandle..>placeholder
   				* @param className ****needed****
   				* @param argument-->placeholder
   				* @param argument_size-->placeholder
   				*/
   				/*call remote nodes to exchange all with the above message structure.*/
                 Message* message = new Message(Message::GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT,
                   							 0, getHandle(), 0, neighborKey,NULL,0);

                vector <MNode *> mNode = MASS::mNodes;
                for(int i = 0; i < (int)mNode.size(); i++){
                	MNode* nod  = mNode.at(i);
                 if(nod->getPid() == owner){

                 	if(printOutput){
			  			ostringstream convert;
			  			convert << "GraphPlaces: \"ExchangeAll\" method --> GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT"
			  			<< " message sent to " << nod->getHostName() << endl;
			  			MASS_base::log(convert.str());
			  		}
                 	nod->sendMessage(message);
                 	if(printOutput){
			  			ostringstream convert;
			  			convert << "GraphPlaces: \"ExchangeAll\" method --> GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT"
			  			<< " waiting for ack  from " << nod->getHostName() << endl;
			  			MASS_base::log(convert.str());
			  		}

                 	message = nod->receiveMessage();

                 	if(printOutput){
			  			ostringstream convert;
			  			convert << "GraphPlaces: \"ExchangeAll\" method --> "
			  			<< " ack received from " << nod->getHostName() << endl;
			  			MASS_base::log(convert.str());
			  		}

                 	result = message->getArgumentPointer();
                 }

                }                     
             }
            place->setNeighborResult(neighborKey, result);
        }
    }
}
}

/**
 *This method helps neighbors to exchange info.

 *given a function and the index of a particular Place object,
 *this method returns the result of excecution of the method.
 *@param functionId 			the unuque ID of the function,
 *@param neighbor 		 		the vector of indices of a particular Place object.
 *@param argument 				a void pointer argument for the function
 *@return 				 		void 

 */

void* GraphPlaces::exchangeNeighbor(int functionId, vector<int> neighbor, void* argument){

	int indexFound = -1;
	VertexPlace* vertex;
	for(int i = 0; i < (int)placesVector.size(); i++){
		vector<VertexPlace*> vp = placesVector.at(i);
		for(int j = 0; j < (int)vp.size(); j++){
			VertexPlace* place = vp.at(j);
			if(neighbor.at(0) == place->index.at(0)){
				indexFound = 0;
				vertex = place;
				break;
			}			
		}
	}
	if(indexFound != -1){
		return vertex->callMethod(functionId, argument);
	}
	return NULL;	
}

/**
 * this method returns a total number of Place instances 
 * on a particular node that this method called on

 */
int GraphPlaces::getTotalPlaceOnThisNode(){
	int total = places_size;
	for(int i = 0; i < (int)placesVector.size(); i++){
		total += placesVector.at(i).size();
	}
	return total ; 

}
/**
 *Given a global linear index, this methd calcualtes where the place with this
  *global linear index lives at
  *@param 				globalLiearIndex of a particular Place object
  *@return  			the node's pid that the given Place object resides

  */
int GraphPlaces::getNodeIdFromGlobalLinearIndex(int globalLinearIndex){
	int total = 1;
	for(int i = 0; i < dimension; i++){
		total *= size[i];
	}
    int strip = total / MASS_base::systemSize;
    int nodePid = (globalLinearIndex % total)/strip;

    return nodePid;	
}

/**
 *This method checks if the given vertex lives in the simulation space or not.
 *@param  vertex 		the name of the vertex
 *@return 				true if the vertex found in the map, false otherwise

 */
bool GraphPlaces::VertexFoundInDistributedMap(string vertex){
	//check if the given vertix is in the distributed map and return true if found false otherwise.
	auto const& exists  = places_base_distributed_map->count(vertex);
	if(exists == 1){
		return true;
	}
	return false;
}


/*given the message, this method sends the message to all remote nodes*/
void GraphPlaces::sendMessageToAllWorkers(Message *message, string action){

	vector <MNode *> mNode = MASS::mNodes;
	
	for(int i = 0; i < (int)mNode.size(); i++){
		MNode* node = mNode.at(i);
		node->sendMessage(message);
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--> " << action<<" message sent to node: " << node->getHostName();
			MASS_base::log(convert.str()); 
		}
	}
}

/*given the pid of a node, the host name will be returned*/
string GraphPlaces::getHostNameFromPid(int pid){
	string host;
	vector <MNode *> mNod = MASS::mNodes;
	for(int i = 0; i < (int)mNod.size(); i++){
		MNode* node = mNod.at(i);
		if(node->getPid() == pid){
			host = node->getHostName();
			return host;
		}
	}

	//if pid not found in MASS::mNodes, it hsould be the master.
	return "";
	
}

void GraphPlaces::getListOfHostsAsAstring(std::vector<string> &vecHost){
	vector <MNode *> mNods = MASS::mNodes;
	for(int i  = 0 ; i < (int)mNods.size(); i++){
	    MNode * nodes = mNods.at(i);
		vecHost.push_back(nodes->getHostName());
	}

}
/**
 *given the global linear index of a Place object, this method returns the Place instace 
  *which lives on this index (coordinate)in the global simulation space
  *@param globalIndex 			the global linear index 
  *@return 						the Place instance which resides on the coordinate passed as an argument

  */
VertexPlace* GraphPlaces::getVertexPlaceFromGlobalLinearIndex(int globalLinearIndex){
	int total = 1;
	for(int i = 0; i < dimension; i++){
		total *= Places_base::size[i];
	}
	int localPlacesIndex = globalLinearIndex / total - 1;
	int strip = total / MASS_base::systemSize;
    int placeIndex = globalLinearIndex % strip;

   VertexPlace* place =  placesVector.at(localPlacesIndex).at(placeIndex);
   return place;
	
}

/**
 *given the vertex name as a string, this method returns the Place(VertexPlace pointer),
 *The place might exist either in the master node or in one of remote nodes, the location
 *of the vertex is hidden from the applciation view point

 */
VertexPlace* GraphPlaces::getPlaceFromVertexName(string vertexname){

	unordered_map<string, int> *myMap = getThisDistributedMap(handle);

	//is the vertex name in the distributed map? if yes, find the place object on the 
	//master node or on the remote nodes, otherwise return NULL.
	std::unordered_map<std::string,int>::const_iterator found = myMap->find (vertexname);
	DllClass * dllclass  = nullptr;
	if(found != myMap->end()){
		
		auto it = MASS_base::dllMap.find(handle);

		DllClass * dllclass = it->second;
		
		for(int i = 0; i < places_size; i++){
			VertexPlace* place = (VertexPlace*)dllclass->places[i];
			if(place->getVertexName() == vertexname){
				return place;
			}
		}			 
	}
	for(auto const& vecOfPlace:placesVector){
		for(auto const &vertexPlace:vecOfPlace){
			if(vertexPlace->getVertexName() == vertexname){
				return vertexPlace;
			}
		}
	}
 
	 /*Message format:
	 *ACTION_TYPE action, int initPopulation, int handle, int placeHandle, 
	 *string className, void *argument,int argument_size*/
	Message* m = new Message (Message::GET_VERTEXPLACE_FROM_VERTEXNAME, 0, this->handle, 0, vertexname, NULL, 0);
	vector <MNode *> mNode = MASS::mNodes;
	
	for(int i = 0; i < (int)mNode.size(); i++){
		MNode* node = mNode.at(i);
		node->sendMessage(m);
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--> GET_VERTEXPLACE_FROM_VERTEXNAME  message sent to: " << node->getHostName();
			MASS_base::log(convert.str()); 
		}
	}
	delete m;
	//VertexPlace(string vertexName, std::vector<string> &neighbors);
	
	/*Response:
		*ACTION_TYPE action,vector<int> *size, int handle, string classname, void *argument,
          int arg_size, int boundary_width, vector<string> *hosts)
        *--->this is a hack to avoid a separate serialization/deserialization implementaion.*/
	Message * message = new Message();
	std::vector<string> neighborVec;
	int id  = -1;
	bool foundFromRemote = false;
	for(int i = 0; i < (int)mNode.size(); i++){
		MNode* node = mNode.at(i);
		message = node->receiveMessage();
		if(printOutput){
			ostringstream convert;
			convert << "GraphPlaces--> GET_VERTEXPLACE_FROM_VERTEXNAME message received from: " << node->getHostName();
			MASS_base::log(convert.str()); 
		}	
		if((message->getHosts()).size() != 0){
			neighborVec = message->getHosts();
			id = message->getHandle();
			foundFromRemote = true;
		}	

	}
	
	if(foundFromRemote){
		VertexPlace* plc = (VertexPlace*)dllclass->places[0];
		plc->setVertexName(vertexname);
		plc->setNeighbors(neighborVec);
		plc->setGlobalId(found->second);
		//delete message;
		return plc;
	}
	//vertex cound't found in the cluster
	return NULL;
}
