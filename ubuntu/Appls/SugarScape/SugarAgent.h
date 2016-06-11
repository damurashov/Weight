//
//  SugarAgent.h
//  Sugarscape
//
//  Created by Sourish Chatterjee on 2/7/15.
//

#ifndef __Sugarscape__Agent__
#define __Sugarscape__Agent__

#include <stdio.h>
#include "Agent.h"
#include "MASS_base.h"
#include <sstream> // ostringstream
#include <vector>
#include <stdlib.h>
const bool printOut = false;
unsigned int seed = (uint)time(NULL);

using namespace std;

class SugarAgent : public Agent {

public:
    static const int markNewPlace_ = 0;
    static const int moveToNewPlace_ = 1;
    static const int metabolize_ = 2;
    static const int initAgent_ = 3;
    static const int postMigrationReset_ = 4;
    static const int showAgent_ = 5;
    static const int randomKill_=6;
    
    // other constants of this class
    static const int maxVisible = 3;
    static const int maxMetabolism = 4;
    static const int maxInitAgentSugar = 10;
    
    ///using Agent::map;
    ///int map(int maxAgents, vector<int> size, vector<int> coordinates);
    
    
    SugarAgent( void *argument ) : Agent( argument ) { };
    
    
    
    
    
    virtual void *callMethod( int functionId, void *argument ) {
        switch( functionId ) {
            case markNewPlace_: return markNewPlace( );
            case moveToNewPlace_: return moveToNewPlace( argument );
            case metabolize_: return metabolize( argument );
            case initAgent_: return initAgent(argument);
            case postMigrationReset_:return postMigrationReset();
            case showAgent_:return showAgent();
            case randomKill_:return randomKill();
        }
        return NULL;
    };
    
    int* getAgentFutureCoordinateAndId();
    int getAgentId();
    
private:
    int visible;
    int metabolism;
    int sugar;
    int absMigX;
    int absMigY;
    
    int dest_getMessage[2];
    double maxRatio;
    
    void *initAgent(void *argument);
    void *postMigrationReset();
    void *markNewPlace( );
    void *moveToNewPlace( void *argument );
    void *metabolize( void *argument );
    void *showAgent();
    void *randomKill();
};

#endif /* defined(__Sugarscape__SugarAgent__) */
