#include "VertexPlace.h"

#include <sstream>  // ostringstream
#include <fstream>//file stream
#include <utility>//std::pairs data structure 
#include <algorithm>//for std::find()
#include <iterator>
//#include <map>
#include <iostream>

//MASS lib includes
#include "MASS_base.h"


// Used to toggle output for DllClass
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif 

//a MASS_base map to hold verices read form a file
//#define map <std::string, int> &distributed_map = MASS_base::distributed_map; 
 
VertexPlace::VertexPlace():Place(NULL){
	this->vertexGlobalIndex = -1;
	this->filepath = "";
	this->vertexName = "";
	this->fType = FILE_TYPE_ENUMS::HIPPIE;//default file type enum

	if(printOutput){
		
		MASS_base::log("GraphPlace instantiated from empty constructor.");
	}
	
}
VertexPlace::VertexPlace(void* argument): Place(argument){
	this->vertexGlobalIndex = -1;
	this->filepath = "";
	this->vertexName = "";
	this->fType = FILE_TYPE_ENUMS::HIPPIE;//default file type enum

	if(printOutput){
		ostringstream convert;
		convert <<"VertexPlace constructored: argument-->" <<(char*)argument <<endl; 
		
		MASS_base::log(convert.str());
	}
}

VertexPlace::VertexPlace(string filepath, FILE_TYPE_ENUMS type, int globalIndex, 
			             void*argument, std::unordered_map<string, int> *dist_map): Place(argument){
	
	this->vertexGlobalIndex = globalIndex;
	this->filepath = filepath;
	this->fType = type;

	bool placeCreated = FileParser::neighbor_init(dist_map, this->filepath, this->fType,
						 this->vertexGlobalIndex, this->neighbors, this->weights, this->vertexName);

	if(!placeCreated){
		ostringstream convert;
		convert.str("");
	    convert << "Place class faild to instantiate, please check the file path. " << endl;
	    //MASS_base::log(convert.str());
		exit(-1);
	}
	if(printOutput){
		string filetype = FileParser::fromEnumToString(this->fType);
		ostringstream convert;
		convert << "GraphPlace instantiated by reading from filepath " << filepath
		<< " with vertex global index " << globalIndex << " vertex name " << this->vertexName
		<<" file enum type " << filetype<< endl;
		MASS_base::log(convert.str());
	}
	
	 /**vector<int> index--> inside the parent chall "Place.h"
     * Is an array that maintains each place’s coordinates. Intuitively,
     * index[0], index[1], and index[2] correspond to coordinates of x, y, and
     * z, or those of i, j, and k.
     */
    index.push_back(vertexGlobalIndex);
    	
}

VertexPlace::VertexPlace(int globalIndex, void*argument):Place(argument){

	this->vertexGlobalIndex = globalIndex;
	this->filepath = "";
	this->vertexName = "";
	this->fType = FILE_TYPE_ENUMS::HIPPIE;//default filetype

	
	/**vector<int> index--> inside the parent chall "Place.h"
     * Is an array that maintains each place’s coordinates. Intuitively,
     * index[0], index[1], and index[2] correspond to coordinates of x, y, and
     * z, or those of i, j, and k.
     */
    index.push_back(vertexGlobalIndex);
    if(printOutput){
    	ostringstream convert;
    	convert << "GraphPlace constructed with global index " << globalIndex << endl;
    	MASS_base::log(convert.str());
    }    	
}


VertexPlace::VertexPlace(string vertexName, std::vector<string> &neighbors, std::vector<double> &weights)
			:Place(NULL){
	
	this->vertexName = vertexName;
	this->neighbors = neighbors;
	this-> weights = weights;

}
VertexPlace::VertexPlace(string vertexName, std::vector<string> &neighbors):Place(NULL){
	this->vertexName = vertexName;
	this->neighbors = neighbors;
}


VertexPlace::~VertexPlace(){

 }


void VertexPlace::prepareForExchangeAll(){


}

//
void VertexPlace::addNeighbor(string neighborName, double weight){
	
	if(std::find(this->neighbors.begin(), this->neighbors.end(), neighborName) == this->neighbors.end()) {
		this->neighbors.push_back(neighborName);

		//std::pair<double, string> wt = make_pair(weight,"");
		this->weights.push_back(weight);
	}

}

//
void VertexPlace::removeNeighbor(string neighborName){

	auto itr = std::find(std::begin( this->neighbors), std::end(this->neighbors), neighborName);

	if (itr != this->neighbors.end()) {

		int index = std::distance(this->neighbors.begin(), itr);

		this->neighbors.erase(neighbors.begin() + index);
		this->weights.erase(weights.begin() + index);	
	}
}
	

//
std::vector<string> VertexPlace::getNeighbors(){
	return this->neighbors;

}

//
std::vector<double> VertexPlace::getWeights(){
	return this->weights;
}

//
int VertexPlace::getGlobalIndex(){
	return this->vertexGlobalIndex;

}
std::string VertexPlace:: getVertexName(){
	return this->vertexName;

}
void VertexPlace:: clearAllNeighborsAndWeight(){
	this->neighbors.clear();
	this->weights.clear();
}


void VertexPlace::setNeighborResult(string &vertex, void* result) {
	
	//find the position of the vertex
	std::vector<string>::iterator it = std::find(neighbors.begin(), neighbors.end(), vertex);
	
	//if found, calculate the index and update the map
	if(it != neighbors.end()){
		int index = std::distance(neighbors.begin(), it);
    	neighborResults.insert({index, result});
    }

  }


//operator overloading
bool VertexPlace::operator==(const VertexPlace& rhs) const{
	if(this->vertexName == rhs.vertexName || this->vertexGlobalIndex == rhs.vertexGlobalIndex){
		return true;
	}
	return false;

}
bool VertexPlace::operator==(VertexPlace* vertexplace) const{
	if(this->vertexName == vertexplace->getVertexName() || this->vertexGlobalIndex == vertexplace->getGlobalIndex()){
		return true;
	}
	return false;
}

bool VertexPlace::operator< (const VertexPlace& rhs) const{
	if(this->vertexName < rhs.vertexName ||this->vertexGlobalIndex < rhs.vertexGlobalIndex){
		return true;
	}
	return false;
}

bool VertexPlace::operator>(const VertexPlace& rhs) const{
	return *this < rhs;
}
void VertexPlace::setVertexName(std::string vertexName){
	this->vertexName = vertexName;
}

