#ifndef FILE_H
#define FILE_H

#include <string>
#include <iostream>
#include <fstream>//file processing
#include <stdio.h> //exit() to exit from program failure

using namespace std;


enum class IO_FILE_TYPE_ENUMS{
	NETCDF,
    TXT
};

class File{

public:

	File(string &filePath, IO_FILE_TYPE_ENUMS &type);
	virtual ~File();
	
	string getFileName();
	IO_FILE_TYPE_ENUMS getFileType();
	string getFilepath();
 	
	virtual bool open(std::ios::openmode &mode) = 0;
	virtual bool close() = 0;
	
protected:
	IO_FILE_TYPE_ENUMS fileType;
	string fileName;
	string filepath;
	int totalPlaces;
	int myTotalPlaces;
	int totalNodes;
	int myNodeId;

	bool fileNotSharedAmongNodes = false;
	bool placesBigerThanFileSize = false;

	virtual bool getNodeInitialReadOffset(int &, int&result);
	virtual int getNodeLastReadOffset(int &);
	
	virtual bool getPlaceInitialReadOffset(int &, int &,int&);	
	virtual int getPlaceLastReadOffset(int &, int &);

private:
	int getNumOfNodeStrips(int &);
	int getNumOfPlacesStripes(int &);



};
#endif