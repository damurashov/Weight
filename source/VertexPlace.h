/*Writen by Elias Alabssie
* July, 2020 */

#ifndef VERTEXPLACE_H
#define VERTEXPLACE_H

//stl lib includes
#include <string>
#include <vector>
//#include <unordered_map>

#include "Place.h" //inheritance
#include "FileParser.h"//for fileType enum

using namespace std;


class VertexPlace: public Place {
	 friend class Places_base;
     friend class Agents_base;
     friend class Agent;

public:
	friend class GraphPlaces;
	//construcotrs
	VertexPlace(string filepath, FILE_TYPE_ENUMS type, int globalIndex, void*argument, std::unordered_map<string, int> *dist_map);
	VertexPlace(int globalIndex, void*argument);
	VertexPlace(void* argument);
	
	VertexPlace(string vertexName, std::vector<string> &neighbors, std::vector<double> &weights);
	VertexPlace(string vertexName, std::vector<string> &neighbors);
	VertexPlace();

	//destructor
	~VertexPlace();
	
	//utility methods
	void prepareForExchangeAll();
	void addNeighbor(string neighborName, double weight);
	void removeNeighbor(string neighborName);
	void clearAllNeighborsAndWeight();

	//void setNeighborResult(string neighbor, Object result);
	std::vector<string> getNeighbors();
	std::vector< double> getWeights();	
	std::string getVertexName();
	int getGlobalIndex();

	void setVertexName(string vertexName);
	void setNeighborResult(string &neighbor, void* result);
            	
	virtual void *callMethod(int functionId, void *argument) = 0;

	//operator overloading
	bool operator==(const VertexPlace& rhs) const;
	bool operator==(VertexPlace* vertexplace) const;
	bool operator< (const VertexPlace& rhs) const;
	bool operator>(const VertexPlace& rhs) const;

private:
	int vertexGlobalIndex;
	string filepath;
	string vertexName;
	FILE_TYPE_ENUMS fType;
	std::vector<string> neighbors;
	std::vector<double> weights;
	unordered_map <int, void*> neighborResults;
	
	/*Warning: 
	 *The following two methods should not be used anywhere except inside GraphPlaces class
	 * as means of reconstructing the Place object from the message received from remote nodes.
	 */
	void setNeighbors(std::vector<string> v){
		this->neighbors = v;
	};

	void setGlobalId(int id){
		this->vertexGlobalIndex = id;
	};
};
#endif

