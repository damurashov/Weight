#ifndef GRAPHPLACES_H
#define GRAPHPLACES_H


#include<string>
#include<vector>

//MASS library includes
#include "Places.h" //inheritance
#include "Graph.h" //inheritance
#include "VertexPlace.h" //data memebr
#include "FileParser.h" //data memebr

//forward declarations
class Message;
class GraphModel;

using namespace std;


class GraphPlaces: public Places, public Graph{

public:
	friend class Place;
	friend class VertexPlace;
	
	GraphPlaces(int handle,string className,int boundary_width,int dimension, string filename,
                     FILE_TYPE_ENUMS type, void* argument, int arg_size);

	GraphPlaces(int handle, string className, int boundary_width,int dimension,void* argument,
				 int argSize,int nVertices);
	
	~GraphPlaces();

 /*--------------Graph interfaces -----------------------------------------------------------------*/
	
	virtual GraphModel* getGraphOnThisNode(); 
	virtual GraphModel* getAllGraphOnTheCluster();
	virtual void setGraph(GraphModel &newGraph);
	virtual bool addEdge(string, string, double);
	virtual bool removeEdge(std::string vertexId, std::string neighborId);
	virtual int addVertex(string vertexId);
	virtual int addVertex(string vertexId, void *argument, int arg_size);
	virtual bool removeVertex(std::string vertexId);
	virtual VertexPlace* getPlaceFromVertexName(string vertexName);
	
	void merge(GraphModel &source, GraphModel &remoteGraphs);
	//
	virtual void callPlaceMethod(int functionId, void* argument);	
	void callAllWithReturns(int functionId, vector<void*> &returns,vector<void*> &arguments);
	void exchangeAll(int currentFunctionId, int handle);
	void* exchangeNeighbor(int functionId, vector<int> neighbor, void*argument);
	int getTotalPlaceOnThisNode();
	VertexPlace* getVertexPlaceFromGlobalLinearIndex(int globalLinearIndex);
	
	//getters
	FILE_TYPE_ENUMS getFileType(){return this->type;};
	std::string getFileName(){return this->filename;};
	std::string getClassName(){return this->className;};
	int getHandle(){return this->handle;};
		
private:
    std::string filename;
    FILE_TYPE_ENUMS type;
    std::string className;
    int nextPlaceIndex = 0;
    int handle;
	GraphModel getRemoteGraphs();	
	//int* getPlacesIndex();

	bool removeEdgeLocally(std::string vertexId, std::string neighborId);
	bool addEdgeLocally(std::string vertexId, std::string neighborId, double weight);	
	int addPlaceLocally(string vertexId, void* argument, int arg_size);
	//int  addVertexLocally(string vertexId, void* argument, int arg_size);
	bool removeVertexLocally(string vertexId);

	int addVertexPlace(string host, string vertexId, void* argument, int arg_size);
	bool validNeighbor(std::string vertexId, std::string neighborId){return false;}; //Dummy 	
	//map<string, int>* getThisDistributedMap(int handle);	
	int getNodeIdFromGlobalLinearIndex(int globalLinearIndex);
	int getNextPlaceIndex(){return this->nextPlaceIndex;};
	std::vector <std::vector<VertexPlace* > > getPlacesVector(){
		return placesVector;
	};
	bool VertexFoundInDistributedMap(string vertex);
	string getHostNameFromPid(int pid);
	void getListOfHostsAsAstring(std::vector<string> &vecHost);
	
	void sendMessageToAllWorkers(Message *message, string action);
	
};
#endif