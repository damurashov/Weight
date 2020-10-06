//
// Created by sarah on 4/8/20.
//

#ifndef TESTPLACE_H
#define TESTPLACE_H

#include <unordered_map>

#include "MASS_base.h"
#include "VertexPlace.h"
#include "FileParser.h"
#include "Place.h"


class TestPlace : public VertexPlace {
public:
    static const int init_ = 0;
    static const int testAgentsOnPlace_ = 1;

    TestPlace(string filename, FILE_TYPE_ENUMS type, int global, void *arg, unordered_map<string, int>*dist_map) 
              :VertexPlace(filename, type, global, arg, dist_map) {
        
    };

    TestPlace(void* argument):VertexPlace(argument){

    };

    TestPlace():VertexPlace(NULL){

    };

    virtual void *callMethod(int funcId, void *arg) {

        switch(funcId) {

            case init_ :
                return init(arg);

            case testAgentsOnPlace_ :
                return testingAgentsOnPlace(arg);
        }
        return NULL;
    };

private:

    void* init(void* showPlaceId);

    void* testingAgentsOnPlace(void* print);

    ostringstream convert;
};


#endif // TESTPLACE_H
