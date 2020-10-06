#include "TxtFile.h"
#include "MASS_base.h"

//constructor 
TxtFile::TxtFile(std::string &filepath, IO_FILE_TYPE_ENUMS &typ ):File(filepath,typ){
	this->filePath = filepath;

					
 }
int TxtFile::numberOfPreparedPlace = 0;
//destructor
TxtFile::~TxtFile(){

}

std::string TxtFile::getFileNameToWriteTo(){
	return this->filenameToWriteTo;

}
void TxtFile::setFileNameToWriteTo(std::string &filename){
	this->filenameToWriteTo = fileName;

}

/*there is no garantee this methods returns non-null contents 
 *untill the file is read by the node. for garanteed contents, 
 *call open method first and then call getAllTextContents() */
std::vector <char> TxtFile:: getAllTextContents(){
	return this->allTextContents;
}

int TxtFile:: getTotalTextSize(){
	return this->allTextContents.size();
}

/*this method opens the filename provided in the construcotr
 *then the method calls approprite helper methods
 *to populate the vecotor field "allTextContents" 
 *for any further calls to access the file contents.
 *if problem occures at any 
 *stage on this process, 
 *the method returns false, true otherwise.*/
bool TxtFile::open(std::ios::openmode &mode){
	if(mode == std::ios_base::in){
		return openFileForRead();
	}
	else if(mode == std::ios::out){
		return openFileForWrite();
	}
	else if(mode == std::ios::app){
		return openFileFOrAppen();
	}
	else{
		MASS_base::log("open mode not supported");
		return false;
	}
}

bool TxtFile::openFileForRead(){

	//allTextContents will be populated with text contents
	bool fileRead = this->readFile(std::ios::in); 
	if(!fileRead){
		return false;
	}
	return true;

}

bool TxtFile::openFileForWrite(){

	bool fileRead  = this->readFile(std::ios::out);
	if(!fileRead){
		return false;
	}

	return true;
}

bool TxtFile:: openFileFOrAppen(){
	bool fileRead = this->readFile(std::ios::app);
	if(!fileRead){
		return false;
	}
	return true;
}


bool TxtFile::readFile(ios::openmode mode){
	this->fileInputStream.open(this->filePath, mode);
	if(!fileInputStream){
		return false;
	}

	if(mode == std::ios::out || mode == std::ios::app){
		this->canModifyFile = true;
	}

	char ch = '\0';
	while(fileInputStream.get(ch)){
		this->allTextContents.push_back(ch);
	}

	return true;
}

bool TxtFile::canFileBeModified(){
	return this->canModifyFile;
}

//this method returns vector<char> read by a place given
// the network-independet index of the place: "placeIndex"
std::vector<char> TxtFile:: read(int &placeIndex){

	std::vector<char> contents;
	bool fileRead;
	if(allTextContents.size() == 0){
		fileRead = this->openFileForRead();
	}
	if(!fileRead){
		MASS_base::log("unable to open file");
		return contents;
	}

	int totalTextSize = allTextContents.size();
	/*bool File::getPlaceInitialReadOffset(int &sizeOfTextFile,int &placeIndex,
											 int &initialOffset)*/
	//int File:: getPlaceLastReadOffset(int &totalTextSize, int &placeIndex)
	int initPlaceOffset;

	bool gotPlaceShare = this->getPlaceInitialReadOffset(totalTextSize, placeIndex, initPlaceOffset);
	if(!gotPlaceShare){
		MASS_base::log("access not allowed for this place");
		return contents;//return empty content
	}

	int lastPlaceOffset = this->getPlaceLastReadOffset(totalTextSize, placeIndex);

	while(initPlaceOffset <= lastPlaceOffset){
		contents.push_back(allTextContents[initPlaceOffset]);
		initPlaceOffset++;

	}
	return contents;
}

//clear the file. helps to remove temporary files
bool TxtFile::removeFile(){
	if(remove(this->filePath.c_str( )) == 0){
		MASS_base::log("File deleted successfully");
		canModifyFile = false;
		return true;
	}
	MASS_base::log("Warning, file deletion faild"); 
	return false;

}

//close the file streams if any
bool TxtFile::close(){
	bool streamOpend = false;
	this->canModifyFile = false;

	//close the input stream if opened
	if(fileInputStream){
		fileInputStream.close();
		streamOpend = true;
		
	}

	//close the output stream if opened
	if(fileOutputStream){
		streamOpend = true;
	}
	return streamOpend;
}

void TxtFile::write(const std::vector<char> &vec, int &placeIndex){
	
	if(!fileOutputStream){
		fileOutputStream.open(this->filenameToWriteTo);
	}

	this->writeDataToFile(vec, placeIndex);
	numberOfPreparedPlace++;;
	if(numberOfPreparedPlace == myTotalPlaces){
		fileOutputStream.clear();
		fileOutputStream.seekp(0);
	}
}

void TxtFile::writeDataToFile(const std::vector<char> &vec, int &placeIndex){
	std::lock_guard <std::mutex> lock (this->mutx);

	int totalFileSize = vec.size();
	//virtual bool getPlaceInitialReadOffset(int &, int &,int&);
	int placeBegin;	
	bool initVal = getPlaceInitialReadOffset(totalFileSize, placeIndex, placeBegin);
	if(!initVal){
		MASS_base::log("no access for the data");
		exit(0);
	}

	int placeEnds = getPlaceLastReadOffset(totalFileSize, placeIndex);
	char ch = '\0';

	while(placeBegin <= placeEnds){
		ch = vec[placeBegin];
		placeBegin++;
		fileOutputStream << ch;
	}

}