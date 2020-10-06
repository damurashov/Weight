#ifndef VERTEXMODEL_H
#define VERTEXMODEL_H

#include <string>
#include <vector>
#include "VertexPlace.h"

using namespace std;


class VertexModel {

public:
	
	VertexModel(string vertexplace, std::vector<string> &neighbors, vector<double> &weight);
	VertexModel(string vertexplace, std::vector<string>& neighbors);
	
	~VertexModel();
	
	std::string getVertexPlaceName();
	std::vector<string> getVertexPlaceNeighbors();
	std::vector<double> getVertexPlaceWeights();

private:
	string vertexPlaceName;
	std::vector<string> neighbors;
	std::vector<double> weights;

	
};
#endif