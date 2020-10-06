/*Writen by Elias Alabssie
* July, 2020 */

#include "FileParser.h"
#include "MASS_base.h"



// Used to enable or disable output in places
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif


FILE_TYPE_ENUMS FileParser::type;



int FileParser::open(std::string filepath, FILE_TYPE_ENUMS ft, std::unordered_map<string, int> *vertexcount_map){
	
	if(printOutput){
		MASS_base::log("FileParser: open() function reached");
	}
	
	switch(ft){
		case FILE_TYPE_ENUMS::CSV:	
			 return parse_csv_file(filepath, vertexcount_map);
			
		case FILE_TYPE_ENUMS::HIPPIE:
			 return parse_hippie_file(filepath,vertexcount_map);
			 
		case FILE_TYPE_ENUMS::MATSim:
			 return parse_matsim_file(filepath,vertexcount_map);
			 //return -1;
			
			
		case FILE_TYPE_ENUMS::TXT:
			 return parse_txt_file(filepath,vertexcount_map);
			
		case FILE_TYPE_ENUMS::FILEGEN:
			 return parse_filegen_file(filepath,vertexcount_map);
			

	}
	//MASS_base::log("input file type not understood");
	return -1;	
}

//-----------------------------------------------------------------------------------------
/* in HIPPIE file input format,the node list and 
the link weight are in the same file*/
int FileParser::parse_hippie_file(std::string filepath, 
				std::unordered_map <std::string, int> * vertex_to_vertexcount_map){

	std::ifstream infile(filepath.c_str());
	if(!infile){
		if(printOutput){
			MASS_base::log("unable to open file");
		}

		return -1;
	}
	
	int count = 0;
	std::string line;
	//std::string vertx;
	while(std::getline(infile, line)){

		std::string vertx;
		std::istringstream iss (line);
		iss >> vertx;

		if( vertx.size() != 0 && !vertex_to_vertexcount_map->count(vertx)){
			vertex_to_vertexcount_map->insert({vertx, count});
			count++;
			
		}

	}
	infile.close();
	if(printOutput){
		ostringstream convert;
		convert <<" FileParser::parse_hippie_file inished. number of vertex read from file=" << count << endl;
		MASS_base::log(convert.str());
	}
	return count;

}


/*input file has the neighbor ids and neighbor weights into
the same file, both combined.*/

int FileParser::parse_matsim_file(std::string filepath, 
	std::unordered_map <std::string, int> * vertex_to_vertexcount_map){

	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file(filepath.c_str());

	if(!result){
		cerr << "unable to open the xml file" << endl; 
		return -1;
	}
	
	std::string name = "node";
	std::string query_string = "//*[local-name(.) = '" + name + "']";

	pugi::xpath_node_set all_nodes;

	try{
		 all_nodes = doc.select_nodes(query_string.c_str());
	}

	catch(pugi::xpath_exception& e) {

   		std::cerr << e.what() << std::endl;
	}

	int count = 0;
	for (pugi::xpath_node node: all_nodes){

	 	pugi::xml_node tool = node.node();

	 		
	 	pugi::xml_attribute attr = tool.first_attribute();
	 	std::string nodeId = attr.value();

	 	if(nodeId != "" && (!vertex_to_vertexcount_map->count(nodeId))){

	 		vertex_to_vertexcount_map->insert({nodeId, count});
	 		count++;

	 	}

    }
    int totalCount = count++;
    if(printOutput){
		ostringstream convert;
		convert <<" number of vertex: " << totalCount << endl;
		MASS_base::log(convert.str());
	}
    return totalCount;


}


/*this method expects one nodes' id/name comma separeted(also space after comma) 
 *implementation of this algorithm expects the list of nodes and list of 
 *links in a separet file.*/
int FileParser:: parse_csv_file(std::string filepath,
	std::unordered_map <std::string, int> * vertex_to_vertexcount_map){

	std::ifstream infile(filepath.c_str());
	if(!infile){
		//MASS_base.log("unable to open file");
		return -1;
	}
	
	int count = 0;
	std::string vertx;
	//std::string vertx;
	while(infile >> vertx){

		//strip the comma out
		if(vertx[vertx.size() - 1] == ','){
			vertx.pop_back();
		}

		if( vertx.size() != 0 && !vertex_to_vertexcount_map->count(vertx)){

			vertex_to_vertexcount_map->insert({vertx, count});
			//cout << vertx << " " << count << endl;
			count++;
		}

	}
	infile.close();
	if(printOutput){
		ostringstream convert;
		convert <<" number of vertex: " << count << endl;
		MASS_base::log(convert.str());
	}

	return count;

}

int FileParser::parse_txt_file(std::string filepath, 
	std::unordered_map <std::string, int> * vertex_to_vertexcount_map){
	//cout << "txt" << endl;
	//todo implement
	return -1;

}


int FileParser::parse_filegen_file(std::string filepath, 
	std::unordered_map <std::string, int> * vertex_to_vertexcount_map){
	//todo implement
	return -1;
	

}



/*--------------------------------------------------------------------------------------------------- 
*---------------------initialize neighbors-----------------------------------------------------------
*----------------------------------------------------------------------------------------------------*/

//neighbor_init(std::string, fileType, int index);
bool FileParser::neighbor_init(std::unordered_map<string, int> *distributed_map, std::string filepath, FILE_TYPE_ENUMS type,int index,
		std::vector<string> &neighbors, std::vector<double> &weight, string &vertexName){
	  if(printOutput){
		ostringstream convert;
		convert <<"FileParser-> neighbor_init function: with distributed_map size= " << distributed_map->size() << endl;
		MASS_base::log(convert.str());
	}
	
   	switch(type){

		case FILE_TYPE_ENUMS::HIPPIE:		
			  			
			return hippie_file_init_neighbor(distributed_map, filepath, index, neighbors,weight, vertexName);

		case FILE_TYPE_ENUMS::MATSim:
			return matsim_file_init_neighbor(distributed_map, filepath, index, neighbors,weight, vertexName);
		//return false;
			

		case FILE_TYPE_ENUMS::CSV:
				//TODO need implementaion				
			return true;

		case FILE_TYPE_ENUMS::TXT: 
			//TODO need implementaion
			return true;

		case FILE_TYPE_ENUMS::FILEGEN:				
			//TODO need implementaion
			return true;
				
		default:
			//MASS_base::log("file type not recognized");
			return false;

		}
	}

//hippie_file_init_neighbor(filepath, index);
bool FileParser:: hippie_file_init_neighbor(std::unordered_map<string, int> *distributed_map, std::string &filepath,
											 int index, std::vector<string> &neighbors, std::vector< double> &weights, 
											 string &vertexName){

	//expected HIPPIE file format example
	/*AL1A1_HUMAN 216 AL1A1_HUMAN 216 0.76 experiments:in vivo,Two-hybrid;pmids:12081471,16189514,
					25416956;sources:HPRD,BioGRID,IntAct,MINT,I2D,Rual05*/
	//open file stream
	std::ifstream inputfile(filepath.c_str());
	if(!inputfile){
		if(printOutput){
			ostringstream convert;
			convert <<"FileParser:: hippie_file_init_neighbor()--> unable to read file name="<< filepath << endl;
			MASS_base::log(convert.str());
		}		
		return false;
	}

	std::string proteinKey = "";
	//look for the protein vertex in the global map
	//+++++++ distributed map is the global map :member of MASS_base

	for(auto const& map:*distributed_map){
		std::string vertex = map.first;
		int globalIndex = map.second;

		if(index == globalIndex){
			proteinKey = vertex;
			break;
		}
	}

	//if protein key not found in the global map, return false.
	if(proteinKey == ""){
		return false;
	}
	vertexName = proteinKey;

	int firstId;
	std::string neighbor;
	int secondId;
	double weight;
	std::string data;

	std::string line;
	
	while(std::getline(inputfile, line)){
		std::istringstream iss (line);
		std::string vertex;
		iss >> vertex;//vertex 
		iss >> firstId;
		iss >> neighbor;
		iss >> secondId;
		iss >> weight;
		//iss >> data;
		//std::string theRest;
		
		//read the rest of line and concatenate them with data
		//while(iss >> theRest){
			//data += theRest;
		//}
		if(vertex == proteinKey){
			//std::pair<double, string> weightData =  make_pair(weight, data);
			neighbors.push_back(neighbor);
			weights.push_back(weight);
		}		
	}

	inputfile.close();
	if(printOutput){
		ostringstream convert;
		convert <<"FileParser:: hippie_file_init_neighbor() finished "
		<< "for " << "Vertexname=" <<vertexName << " #neighbors="<< neighbors.size()  << endl; 
		MASS_base::log(convert.str());
	}

   return true;
}


//expected MATSim file format
	/*<network>
		<nodes>
			<node id="1" x="2000" y="1000" />
			....
		</nodes>

	<links capperiod="01:00:00">
		<link id="1" from="1" to="2" length="3000.0" capacity="1800" freespeed="13.88" permlanes="1" modes="car" />
		....
	</links>
</network>*/


bool FileParser::matsim_file_init_neighbor(std::unordered_map<string, int> *distributed_map, std::string &filepath, 
											int index, std::vector<string> &neighbors, std::vector<double> &weights,
											 string &vertex){
	if(printOutput){
		ostringstream convert;
		convert << "FileParser: matsim_file_init_neighbor function. size of distributed_map " 
		<< distributed_map->size() << endl;
		 MASS_base::log(convert.str());
	}

	//cerr << "MATSim parser called. " << endl;
	std::string proteinKey = "";
	//look for the protein vertex in the global map
	//+++++++ distributed map is the global map: member of MASS_base
	for(auto const& map:*distributed_map){
		std::string vertexName = map.first;
		int globalIndex = map.second;

		if(index == globalIndex){
			//cerr << vertexName << " found" << endl;
			proteinKey = vertexName;
			break;
		}
	}

	//if protein key not found in the global map, return false.
	if(proteinKey == ""){
		return false;
	}
	vertex = proteinKey;
//-----------------------------------------------------------------------------------------------
	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file(filepath.c_str());

	if(!result){
		//cerr << "unable to open the xml file" << endl; 
		return false;
	}
	
	std::string name = "link";
	std::string query_string = "//*[local-name(.) = '" + name + "']";

	pugi::xpath_node_set all_nodes;

	try{
		 all_nodes = doc.select_nodes(query_string.c_str());
	}

	catch(pugi::xpath_exception& e) {

   		std::cerr << e.what() << std::endl;
   		//return false;
	}

	//<link id="1" from="1" to="2" length="3000.0" capacity="1800" freespeed="13.88" permlanes="1" modes="car" />
	for (pugi::xpath_node node: all_nodes){

	 	pugi::xml_node tool = node.node();

	 	std::string vertexName = tool.attribute("from").value();
    	if(proteinKey == vertexName){
    		string neighborVertex = tool.attribute("to").value();
    		double weight = tool.attribute("length").as_double();

    		neighbors.push_back(neighborVertex);
    		weights.push_back(weight);
	 	}	 	
    }
    if(printOutput){
		ostringstream convert;
		convert <<"FileParser:: MATSim_file_init_neighbor() finished "
		<< "for " << "Vertexname=" <<vertex << " #neighbors="<< neighbors.size()  << endl; 
		MASS_base::log(convert.str());
	}
    return true;
}



/*
void FileParser::csv_file_init_neighbor(std::ifstream &stream,  int &vertexName,
					std::vector<std::tuple<std::string, double, std::string>> &adjacencyList){

	
 }

 
void FileParser::txt_file_init_neighbor(std::ifstream &stream,  int &vertexName,
					std::vector<std::tuple<std::string, double, std::string>> &adjacencyList){

	
 }

void FileParser::filegen_file_init_neighbor(std::ifstream &stream,  int &vertexName,
					std::vector<std::tuple<std::string, double ,std::string>> &adjacencyList){

	
 }
*/

/*change the given enum class into the corresponding
 *string representation.*/
std::string FileParser::fromEnumToString(FILE_TYPE_ENUMS type){
	string filenameType;// = "";
	switch(type){
		case FILE_TYPE_ENUMS::CSV:	
			 filenameType = "CSV";
			 return filenameType;
			
		case FILE_TYPE_ENUMS::HIPPIE:
			 filenameType = "HIPPIE";
			 return filenameType;
			 
		case FILE_TYPE_ENUMS::MATSim:
			filenameType  = "MATSim";
			 return filenameType;
			
			
		case FILE_TYPE_ENUMS::TXT:
			 filenameType  = "TXT";
			 return filenameType;
			
		case FILE_TYPE_ENUMS::FILEGEN:
			 filenameType  = "FILEGEN";
			 return filenameType;
	}
	return filenameType = "";
}

//convert the string filetype into the enum fileTyp types
//the default is HIPPIE
FILE_TYPE_ENUMS FileParser::fromStringToEnum(string fType){
	//fileType type;
	if(fType == "HIPPIE" ) return FILE_TYPE_ENUMS::HIPPIE;
	if(fType == "MATSim") return FILE_TYPE_ENUMS::MATSim;
	if(fType ==  "CSV") return FILE_TYPE_ENUMS::CSV;
	if(fType == "TXT") return FILE_TYPE_ENUMS::TXT;
	if(fType == "FILEGEN") return FILE_TYPE_ENUMS::FILEGEN;
	if(printOutput){
		MASS_base::log("FileParser::fromStringToEnum()--> unrecognized enum used for the file type enum.");
	}
	exit(-1);
}
