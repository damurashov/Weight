#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include "GraphModel.h"

using namespace std;



class Graph {
public:
	Graph(){};
	virtual ~Graph(){};
	virtual GraphModel* getGraphOnThisNode() = 0;
	virtual GraphModel* getAllGraphOnTheCluster() = 0;
	 virtual void setGraph(GraphModel &newGraph) = 0;
	
	//Graph maintenance features
	virtual bool addEdge(string vertexId, string neighborId, double weight) = 0;
   	virtual bool removeEdge(string vertexId, string neighborId) = 0;

    virtual int addVertex(string vertexId) = 0;
    virtual int addVertex(string vertexId, void* argument,int arg_size) = 0;
    virtual bool removeVertex(string vertexId) = 0;
   

	
}; 
#endif