#include "GraphPlaces.h"
#include "MASS.h"
#include "Places.h"
#include "Test.h"
#include "TestPlace.h"
#include "Message.h"
#include "FileParser.h"
#include <string>
using namespace std;

int main( int argc, char *argv[] ) {	
	
	char *arguments[4] = {"alabssie", "@workiealabssieLEMMA12uw", "machinefile.txt", "58539"};
    int nProc = 3;
    int numThreads = 1;    
    int simSize [2];// simulation space         
	simSize[0] = 1000;
    simSize[1] = 1000;
	MASS::init(arguments, nProc, numThreads);

	cout << "\n\n" ;
 	 	
	char* msg =  (char*)("argument\0");
    std::string s1 = "SNX12_HUMAN";
    std::string s2 =  "FYN_HUMAN";
       	
	cerr <<"Before Graphplaces constructor ------------------------------------------------------------------->\n" << endl;
    //Places *testPlaces = new Places(1, "Test", 1, (void*)msg, strlen(msg) + 1, 2, simSize);
     //GraphPlaces *gpalces = new GraphPlaces(3, "TestPlace", 1, 1, msg, sizeof(msg), 1000);
	GraphPlaces *places = new GraphPlaces(2,"TestPlace", 1, 1, "hippie_current.txt", FILE_TYPE_ENUMS::HIPPIE,(void*)msg, sizeof(msg) + 1);

    // bool b = places->addEdge(s1, s2, 2.0);
    // cout << "Edge Added ? = " << b << endl;
    // bool remove = places->removeEdge(s1, s2);
    // cout << "Remove edge = " << remove << endl;
    // int add  = places->addVertex("Elias_12");
    // cout << "Add vertex  = " << add << endl;

    //bool removeVertex = places->removeVertex(s1);
    VertexPlace* p = places->getPlaceFromVertexName(s1);
    cout << "Remove vertex  = " << "vertex name = " << p->getVertexName() << endl;
   
  	
    //delete testPlaces;
    //delete places;
    //delete gpalces;

    cerr <<"After Graphplaces constructor--------------------------------------------------------------------->\n\n" << endl;
	MASS::finish( );
    
	return 0;
}
