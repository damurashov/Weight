#include "Agent.h"
#include "MASS_base.h"
#include <iostream>
#include <sstream>     // ostringstream

//Set number for spawning additional Agents
void Agent::spawn( int numAgents, vector<void*> arguments, int arg_size){

	//Only want to make changes if the number to be created is above zero
	if(numAgents > 0){
		newChildren = numAgents;
		this->arguments = arguments;
	}
}

//Set index for an Agent to migrate to
bool Agent::migrate( vector<int> index ){
	vector<int> sizeVectors;
	sizeVectors = place->getSizeVect();
	for(int i = 0; (unsigned)i < sizeVectors.size() - 1; i++){
		if(index[i] >= 0 && index[i] < sizeVectors[i])
			continue;
		else
			return false;
	}
	this->index = index;
	return true;
}
