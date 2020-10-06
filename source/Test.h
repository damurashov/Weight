//
// Created by sarah on 4/8/20.
//

#ifndef TESTPLACE_H
#define TESTPLACE_H


#include <Place.h>
#include "MASS_base.h"

class Test : public Place {
public:
    static const int init_ = 0;
    static const int testAgentsOnPlace_ = 1;

    virtual void *callMethod(int funcId, void *arg) {

        switch(funcId) {

            case init_ :
                return init(arg);

            case testAgentsOnPlace_ :
                return testingAgentsOnPlace(arg);
        }
    }

    Test(void *arg) : Place(arg) {}

private:

    void* init(void* showPlaceId);

    void* testingAgentsOnPlace(void* print);

    ostringstream convert;
};


#endif // TESTPLACE_H
