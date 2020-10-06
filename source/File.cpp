
#include "File.h"
#include "Places_base.h"
#include "MASS_base.h"//for logger message


// Used to toggle comments from Places.cpp
#ifndef LOGGING
const bool printOutput = true;
#else
const bool printOutput = true;
#endif

File::File(string & filePath, IO_FILE_TYPE_ENUMS &type){
	this->filepath = filePath;
	this->fileType = type;
	Places_base *pbase = MASS_base::getCurrentPlaces();
    this->totalPlaces = pbase->getTotalPlaces();
    this->totalNodes = MASS_base::systemSize;
    
    this->myNodeId = MASS_base::getMyPid();

    if(printOutput){
    	ostringstream convert;
    	convert << "your node id = " << myNodeId << ", total numbe of nodes = " << totalNodes
    	<< ", total places = " << totalPlaces << ", total places for this node = " << myTotalPlaces << endl;	
		MASS_base::log( convert.str( ) );
	}

 }

 //dest
 File::~File(){

 }

string File::getFileName(){
	return fileName;

}

//get the file type of the opened file
IO_FILE_TYPE_ENUMS File::getFileType(){
	return fileType;

}


//this method returns the number of file blocks for each node in the cluster
int File::getNumOfNodeStrips(int &totalFileSize){
		int nodeStrips =  totalFileSize / this->totalNodes;

		//if file size is less than #nodes, no need to share among nodes
		//places on a single node can handle it since size is too small
		if(nodeStrips < 1){
			MASS_base::log("File size is too small to be shared: file will be processed by rank 0");
			this->fileNotSharedAmongNodes = true;
			return nodeStrips;
		}

		this->fileNotSharedAmongNodes = false;
		return nodeStrips;
}

int File::getNumOfPlacesStripes(int & TotalSizeOfFile){
	
	int placeStrip = TotalSizeOfFile/ myTotalPlaces;

	/*if the block size is smaller than total places on the node,
	 *simply share the file size for #nodes = sizeOfFileOnNode % myTotalPlaces
	 *and mark the index of the last node that shares the file.
	 *nodes only from [0 to sizeOfFileOnNode % myTotalPlaces] are participating in
	 *reading/writing in this file
	 */
	if(placeStrip < 1){
		int participatingPlaces = TotalSizeOfFile % myTotalPlaces;

		this->placesBigerThanFileSize = true;
		return (participatingPlaces - 1);

	}
		
	return placeStrip;
}

//this method calculates the initial block of the file for a particular
//node to start reading from
bool File::getNodeInitialReadOffset(int &sizeOfTextFile, int &resultIndex){

	int nodeStrips = this->getNumOfNodeStrips(sizeOfTextFile);

	//if file is too small to share by multiple nodes, only the master node has access to the file
	//that way the communication overhead is elliminated by processing the file locally(on master node) 
	if(this->fileNotSharedAmongNodes && myNodeId != 0){
		MASS_base::log("node not allowed accessing this file");
		resultIndex = sizeOfTextFile + 1;//which is out side of the file size to avoid
										//unwilling nodes accessing the file
		return false;
	}
	
	if(this->fileNotSharedAmongNodes && myNodeId == 0){
		resultIndex = 0;
		return true;
	}

	resultIndex = myNodeId * nodeStrips; 

	return true;	
}

int File::getNodeLastReadOffset(int &totalTextSize){
	
	int strips = this->getNumOfNodeStrips(totalTextSize);

	if(fileNotSharedAmongNodes){
		if(myNodeId == 0){
			return totalTextSize - 1;
		}
		else{

			int outOfBlockSignal = totalTextSize + 1;
			return outOfBlockSignal;
		}
	}
	//getNodeInitialReadOffset(int &sizeOfTextFile, int &resultIndex){
	int initOffset;
	bool hasVal = this->getNodeInitialReadOffset(totalTextSize,initOffset);
	if(!hasVal){
		MASS_base::log("illegal access");
		exit(1);
	}
	
	int end =  initOffset + strips - 1;
	if(myNodeId == totalNodes - 1){
		end = totalTextSize - 1;
	}

	return end;

}

bool File::getPlaceInitialReadOffset(int &sizeOfTextFile, int &placeIndex, int &initialOffset){
	
	int nodeStrips = this->getNumOfNodeStrips(sizeOfTextFile);
	if(fileNotSharedAmongNodes && myNodeId != 0){
		initialOffset = sizeOfTextFile + 1;//purposely out of bound number returned.
		return false;
	}
	if(fileNotSharedAmongNodes && myNodeId == 0){
		int placesStrips = sizeOfTextFile/myTotalPlaces;

		initialOffset = placeIndex * placesStrips;
		return true;
	}

	int placesStrips = this->getNumOfPlacesStripes(nodeStrips);

	//a place trying to access a file block outside of its scope
	if(this->placesBigerThanFileSize && placeIndex > placesStrips){
		initialOffset = sizeOfTextFile + 1;//purposely out of file size number returned
		return false;
	}

	//if file is smaller than #nodes, node with rank 0 will process it
	//no need of parallelizing IO operation among nodes since it is too small. 
	initialOffset = placesStrips * placeIndex;

	return true;	
}


int File:: getPlaceLastReadOffset(int &totalTextSize, int &placeIndex){
	
	int initPlaceOffset;
	this->getPlaceInitialReadOffset(totalTextSize, placeIndex, initPlaceOffset);

	int placesStrips = this->getNumOfPlacesStripes(totalTextSize);
	int nodeLastReadOffset = this->getNodeLastReadOffset(totalTextSize);

	if(this->placesBigerThanFileSize && placeIndex > nodeLastReadOffset){
		MASS_base::log("illegal acess of file block");
		exit(1);
	}

	int end = initPlaceOffset + placesStrips - 1;

	return end;
}
