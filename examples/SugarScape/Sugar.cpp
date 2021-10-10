//
//  main.cpp
//  Sugarscape
//
//  Created by Sourish Chatterjee on 2/6/15.
//

#include "MASS.h"
#include <unistd.h>
#include <stdlib.h> // atoi
#include "Timer.h"
#include <vector>
#include "SugarPlace.h"
#include "SugarAgent.h"


using namespace std;

int main(int argc, char * argv[])
{
    if ( argc != 11 )
    {
        cerr << "Usage: ./sugar username password machinefile port nProc numThreads #agents maxSimTime simSize [show]" << endl;
        return -1;
    }
    
    char *arguments[4];
    arguments[0] = argv[1]; // username
    arguments[1] = argv[2]; // password
    arguments[2] = argv[3]; // machinefile
    arguments[3] = argv[4]; // port
    int nProc = atoi( argv[5] );
    int numThreads = atoi( argv[6] );
    int nAgents = atoi(argv[7]);    // # agents
    int maxSimTime = atoi( argv[8] );  // simulation time
    int simSize = atoi( argv[9] );     // simulation space
    bool show = ( argv[10][0]== 's' );
    
    
    Timer timer;
    
    MASS::init( arguments, nProc, numThreads );
    
    
    // common variables initialization
    //CommonVar common( simSize, maxSimTime );
    
    // initialize places
    char *msg = (char *)("placescreation\0");
    
    Places *places = new Places( 1, "SugarPlace", 1, msg, 16, 2, simSize, simSize );
    
    void *argsPlace = new int();
    *(int*)argsPlace = 4; //maxSugar in mountain peak
    
    //TODO:: call all for initPlaces
    places->callAll( SugarPlace::init_, argsPlace, sizeof(int) );
    
    if(show)
    {
        places->callAll( SugarPlace::showSugar_ );
    }
    
    
    int maxVisible = 3;
    void *agentArgs = new int();
    *(int *)agentArgs = maxVisible; // maximum visibility of an agent
    
    std::cout << "Creating {" << nAgents << "} agents now..." << std::endl;
    
    // agent injection
    Agents *agents = new Agents( 2, "SugarAgent", agentArgs, sizeof(int), places, nAgents );
    
    std::cout << "Done creating {" << nAgents << "} agents now..." << std::endl;
    
    // killing some random agents to give the 2D grid a random notion
    agents->callAll(SugarAgent::randomKill_);
    agents->manageAll();
    
    std::cout << "Sugarscape starting with "<< agents->nAgents()<<" agents" << std::endl;
    
    agents->callAll(SugarAgent::initAgent_, agentArgs, sizeof(int));
    
    // show agent positions
    if (show)
    {
        agents->callAll( SugarAgent::showAgent_);
    }
    
    // Start Timer
    timer.start();
    
    for ( int t = 0; t < maxSimTime; t++ )
    {
        // increase sugar and pollution
        places->callAll( SugarPlace::incSugarAndPollution_ );
        
        // calculate average pollution
        places->callAll( SugarPlace::setOutPollution_ ); // broadcasting pollution
        places->exchangeBoundary( );
        places->callAll(SugarPlace::avePollutions_);
        
        // update pollution of each place to their average pollution value
        places->callAll(SugarPlace::updatePollutionWithAverage_);
        
        if (show)
        {
            places->callAll( SugarPlace::showSugar_);
        }
        
        // agent simulation. Migrate agents to a visible place which is empty and then metabolize
        
        // first broadcast each places agent size
        places->callAll(SugarPlace::setOutAgentCountAndSugPolRatio_);
        
        // based on which places are fit for migration agents are marked with their target..
        // ..migration places which will again be verified for duplicates
        agents->callAll(SugarAgent::markNewPlace_);
        
        
        // after agents are set with their target, places broadcast their agent..
        // ..migration data for target places to read and decide
        places->callAll(SugarPlace::prePlaceExchangeAllSetMigData_);
        
        
        
        
        // All possible target destination places from a given place
        vector<int*> migrationDestinations;
        for(int i=1;i<=maxVisible; i++ )
        {
            int *hDest = new int[2];
            hDest[0] = i;
            hDest[1] = 0;
            migrationDestinations.push_back(hDest);
        }
        for(int i=1;i<=maxVisible; i++ )
        {
            int *vDest = new int[2];
            vDest[0] = 0;
            vDest[1] = i;
            migrationDestinations.push_back(vDest);
        }
        
        places->exchangeAll(1, SugarPlace::setOutNextAgentId_, &migrationDestinations);
        
        
        // now agents will be marked for migration based on if their new place is still fit for migration
        agents->callAll(SugarAgent::moveToNewPlace_);
        std::cout << "After moveToNewPlace(migration)..." << std::endl;
        
        agents->manageAll();
        
        // resetting various migration related flags of agents and places
        agents->callAll(SugarAgent::postMigrationReset_);
        places->callAll(SugarPlace::postMigrationReset_);
        
       
        // metabolize agents and kill whoever has no sugar
        agents->callAll(SugarAgent::metabolize_);
        agents->manageAll();
        
        if ( show )
        {
            agents->callAll( SugarAgent::showAgent_);
        }
        
        printf("Agents remaining = '%i' \n", agents->nAgents());
    }
    
    long elaspedTime1 =  timer.lap();
    printf( "\nEnd of simulation. Elasped time using MASS framework with %i processes and %i thread :: %ld \n",nProc,numThreads, elaspedTime1);
    
   MASS::finish( );
   
}


