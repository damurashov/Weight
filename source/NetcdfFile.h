#ifndef NetcdfFile_h
#define NetcdfFile_h

#include "File.h"

class NetcdfFile: public File{
public:

	//this class needs implemenation
	NetcdfFile(std::string &filepath, IO_FILE_TYPE_ENUMS & ioType):File(filepath, ioType){

	};
	~NetcdfFile(){

	};

	//this needs to be implemented
	virtual bool open(std::ios::openmode &mode){
		return true;//dummy return to appease the compiler

	};//need implementation
	virtual bool close() {
		return true;//dummy return to apease the compiler

	}; //need implementaion

private:
	
	
};
#endif