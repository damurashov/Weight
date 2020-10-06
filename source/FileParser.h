#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>// string stream
#include <fstream>//file stream
#include <utility>//std::pairs data structure 

#include "pugixml.hpp"//third party library for parsering xml files

//#include "Places_base.h"


using namespace std;


enum FILE_TYPE_ENUMS{
	CSV,
	HIPPIE,
	MATSim,
	TXT,
	FILEGEN
};
 
class FileParser{

public:
	//friend class Places_base
	 static std::string fromEnumToString(FILE_TYPE_ENUMS type);
	 static FILE_TYPE_ENUMS fromStringToEnum(string fType);
	 static FILE_TYPE_ENUMS type;
	
	//read the vetices into a MASS_base::distributed_map
	 static int open(std::string, FILE_TYPE_ENUMS, std::unordered_map<std::string, int>*);

	 //reads neighbors and the associated weight into vectors for a vetex
	 static bool neighbor_init(std::unordered_map<string, int> * distributed_map, std::string filepath, FILE_TYPE_ENUMS type,int index, 
	 							std::vector<string> &neighbors, std::vector< double> &weight, string&);

	//initialize the vertex ids
	//-------------------------------------------------------------------------------------------
	 static int parse_hippie_file(std::string filepath,std::unordered_map <std::string, int>*);
	 static int parse_matsim_file(std::string filepath,std::unordered_map <std::string, int>*);
	 static int parse_csv_file(std::string filepath,std::unordered_map <std::string, int>*);	
	 static int parse_txt_file(std::string filepath,std::unordered_map <std::string, int>*);
	 static int parse_filegen_file(std::string filepath, std::unordered_map <std::string, int>*);
	 //------------------------------------------------------------------------------------------

	 //initialize neighbirs from a particular input file type
	 static bool hippie_file_init_neighbor(std::unordered_map<string, int> *distributed_map, std::string &, int index,
	 									 std::vector<string> &neighbors, std::vector<double> &waight,string&);
	 static bool matsim_file_init_neighbor(std::unordered_map<string, int> *distributed_map, std::string &, int index,
	 										std::vector<string> &neighbors, std::vector<double> &weight,string &);

	 //TODO implment these
	 
	 /*static bool csv_file_init_neighbor(std::ifstream &stream, int &vertexId, 
	 		std::vector<std::tuple<std::string,double,std::string>>&);

	 static bool txt_file_init_neighbor(std::ifstream &stream, int &vertexId, 
	 		std::vector<std::tuple<std::string,double,std::string>>&);

	 static bool filegen_file_init_neighbor(std::ifstream &stream, int &vertexId, 
	 		std::vector<std::tuple<std::string,double,std::string>>&);	*/
};
#endif