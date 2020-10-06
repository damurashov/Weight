#include "VertexModel.h"

// Used to enable or disable output in places
#ifndef LOGGING
const bool printOutput = true;
#else
const bool printOutput = true;
#endif

VertexModel::VertexModel(string vertexplace, std::vector<string> &neighbors, vector<double> &weight){
	this->vertexPlaceName = vertexplace;
	this-> neighbors = neighbors;
	this-> weights = weight;
}
VertexModel::VertexModel(string vertexplace, std::vector<string>& neighbors){
	this->vertexPlaceName = vertexplace;
	this-> neighbors = neighbors;

}


VertexModel::~VertexModel(){
	neighbors.clear();
	weights.clear();

}

string VertexModel::getVertexPlaceName(){
	return this->vertexPlaceName;

}
std::vector<string> VertexModel::getVertexPlaceNeighbors(){
	return this->neighbors;

}
std::vector<double> VertexModel::getVertexPlaceWeights(){
	return this->weights;
}
