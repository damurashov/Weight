
#ifndef TXTFILE_H
#define TXTFILE_H

#include "File.h"

#include <fstream>
#include <iostream>
#include <vector> 
#include <mutex>//for file write synchronization
#include <cstdio>//for file removal
#include <thread>//lock_gard

using namespace std;

class TxtFile :public File{

public:

	//const and dest
	TxtFile(std::string &, IO_FILE_TYPE_ENUMS &);
	~TxtFile();

	int getTotalTextSize();

	std::vector <char> getAllTextContents();

	//setter and getter for file name to write the data on
	std::string getFileNameToWriteTo();
	void setFileNameToWriteTo(std::string &filename);

	//open file
	virtual bool open(std::ios::openmode &mode);

	bool canFileBeModified();

	//read for a places
	std::vector<char> read(int &placeIndex);
	
	//remove temporary files
	bool removeFile();

	std::vector<char> getThisNodeFileShare();

	void write(const std::vector<char> &vec, int &placeIndex);

	virtual bool close();
	//virtual File* factory(const string &filepath, File*);

private:
	//stream objects and filename
	std::ifstream fileInputStream;
	std::ofstream fileOutputStream;
	std::string filePath;

	//private utility methods
	bool openFileForWrite();
	bool openFileForRead();
	bool openFileFOrAppen();
	bool readFile(ios::openmode mode);
	void writeDataToFile(const std::vector<char> &vec, int &placeIndex);

	//Private memeber fields
	std::vector <char> allTextContents;
	std::vector<char> nodesTextSize;
	bool canModifyFile = false;
	std::mutex mutx;
	static int numberOfPreparedPlace;

	//a file name for the node to write a data to
	//it is the responsiblity of the application developer to rename to what ever.
	std::string filenameToWriteTo; //= "file" + std::to_string(myNodeId) + ".txt";
		
};
#endif