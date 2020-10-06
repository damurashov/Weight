#include "GraphModel.h"


// Used to enable or disable output in places
#ifndef LOGGING
const bool printOutput = true;
#else
const bool printOutput = true;
#endif

GraphModel::GraphModel(){
	this->graphName = "";
	
}

GraphModel::GraphModel(std::string& name, std::vector<VertexModel*> &vertices){

	this->graphName = name;
	this->graphVertices = vertices;
}

GraphModel::~GraphModel(){
	for(auto vrtice:graphVertices){
		delete vrtice;
	}
	graphVertices.clear();
}


std::string GraphModel::getGraphName(){
	return this->graphName;

}

void GraphModel::setGraphName(string& name){
	this->graphName = name;

}

std::vector<VertexModel*> GraphModel::getGraphVertices(){
	return this->graphVertices;
}
void GraphModel::setGraphVertices(std::vector<VertexModel*> &model){
	this->graphVertices = model;

}

void  GraphModel::addVertexModel(string &vertexplace, std::vector<string>& neighbors){
	VertexModel *newModel = new VertexModel(vertexplace,neighbors);
	graphVertices.push_back(newModel);
}
void GraphModel::addVertexModel(string vertexplace, std::vector<string>& neighbors, std::vector<double>& weights){
	VertexModel *newModel = new VertexModel(vertexplace,neighbors, weights);
	graphVertices.push_back(newModel);

}