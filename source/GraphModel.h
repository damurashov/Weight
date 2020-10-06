#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include<string>
#include<iostream>
#include<vector>

#include "VertexModel.h"

using namespace std;


class GraphModel{
	
public:
	GraphModel(std::string&, std::vector<VertexModel*> &);
	GraphModel();
	~GraphModel();

	//getters
	std::string getGraphName();
	std::vector<VertexModel*> getGraphVertices();
	
	//setters 
	void setGraphName(string&);	
	void setGraphVertices(std::vector<VertexModel*> &model);

	//utility methods
	void addVertexModel(string & vertexplace, std::vector<string>& neighbors);  
	void addVertexModel(string vertexplace, std::vector<string>& neighbors, std::vector<double>& weights);      

private:

	std::string graphName;
    std::vector<VertexModel*> graphVertices;
};
#endif