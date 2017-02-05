//
//  SugarAgent.cpp
//  Sugarscape
//
//  Created by Sourish Chatterjee on 3/14/15.
//

#include "SugarAgent.h"
#include <stdlib.h>
#include "MASS_base.h"
#include "SugarPlace.h"

extern "C" Agent* instantiate( void *argument ) {
    return new SugarAgent( argument );
}

extern "C" void destroy( Agent *object ) {
    delete object;
}

unsigned int seedNum = (uint)time(NULL);
bool showLog = false;

//int map(int maxAgents, vector<int> size, vector<int> coordinates )
//{
//    if(coordinates[1]== 0 )
//    {
//        return 1;
//    }
//    else
//    {
//        return 0;
//    }
//}
//int map(int maxAgents, vector<int> size, vector<int> coordinates)
//{
//    MASS_base::log( "Inside new agent map()");
//    //return ( ( rand_r(&seedNum) % 16 ) < 4 ) ? 1 : 0;
//    return 2;
//}

void *SugarAgent::initAgent(void *argument)
{
    int maxVis = *(int*)argument;
    visible = rand_r(&seedNum) % maxVis + 1;
    metabolism = rand_r(&seedNum) % maxMetabolism + 1;
    sugar = rand_r(&seedNum) % maxInitAgentSugar + 1;
    
    absMigY = -1;
    absMigX = -1;
    
    return NULL;
}

void *SugarAgent::randomKill()
{
    if(agentId % 3 ==0 ){
        kill();
    }
    return NULL;
}


void *SugarAgent::markNewPlace()
{
    absMigX = -1; absMigY = -1; maxRatio = 0.0;
    dest_getMessage[0] = -1;
    dest_getMessage[1] = -1;
    
    int currX = index[0];
    int currY = index[1];
    
    int *futureAgentArgs = new int[3];
    futureAgentArgs[0] = agentId;
    
    // considering visibility along x axis
    dest_getMessage[1] = 0; // No displacement along Y axis
    absMigY = currY;
    for (int i = 1; i<=visible; i++)
    {
        int x = ( place->index[0] + i ) % place->size[0];
        
        dest_getMessage[0] = i;

        int *h_remotePlaceInfo = (int*)(place->callMethod(SugarPlace::getRemotePlaceInfo_,dest_getMessage));
        if(h_remotePlaceInfo == NULL)
        {
            continue;
        }
        
        int agentsPresentAtRemotePlace = h_remotePlaceInfo[0];
        int sugarPollutionRatio = h_remotePlaceInfo[1];
        if(agentsPresentAtRemotePlace == 0 and sugarPollutionRatio == 1)
        {
            absMigX = x;
            
            futureAgentArgs[1] = dest_getMessage[0];
            futureAgentArgs[2] = dest_getMessage[1];
            
            place->callMethod(SugarPlace::setAgentFutureCoordinateAndId_,futureAgentArgs);
            
            return NULL;
        }
    }
    
    // considering visibility along Y axis
    dest_getMessage[0] = 0; // No displacement along X axis
    absMigX = currX;
    for (int i = 1; i<=visible; i++)
    {
        int y = ( place->index[1] + i ) % place->size[0];
        
        dest_getMessage[1] = i;
        
        int *v_remotePlaceInfo = (int*)(place->callMethod(SugarPlace::getRemotePlaceInfo_,dest_getMessage));
        if(v_remotePlaceInfo == NULL)
        {
            continue;
        }
        
        int agentsPresentAtRemotePlace = v_remotePlaceInfo[0];
        int sugarPollutionRatio = v_remotePlaceInfo[1];
        if(agentsPresentAtRemotePlace == 0 && sugarPollutionRatio == 1)
        {
            absMigY = y;
            futureAgentArgs[1] = dest_getMessage[0];
            futureAgentArgs[2] = dest_getMessage[1];
            
            place->callMethod(SugarPlace::setAgentFutureCoordinateAndId_,futureAgentArgs);
            return NULL;
        }
    }
    
    // no places found to migrate. Setting dest to -1 ,-1
    dest_getMessage[0] = -1;
    dest_getMessage[1] = -1;
    
    futureAgentArgs[1] = dest_getMessage[0];
    futureAgentArgs[2] = dest_getMessage[1];
    
    place->callMethod(SugarPlace::setAgentFutureCoordinateAndId_,futureAgentArgs);
    
    return NULL;
}


void *SugarAgent::postMigrationReset()
{
    dest_getMessage[0] = -1;
    dest_getMessage[1] = -1;
    absMigX = -1;
    absMigY = -1;
    
    return NULL;
}

void *SugarAgent::moveToNewPlace(void *argument )
{
    ostringstream convert;
    int *remote_targetAgentId = (int*)(place->callMethod(SugarPlace::getRemotePlaceFutureAgentId_,dest_getMessage));
    
    if(remote_targetAgentId == NULL)
    {
        if(showLog){
            convert << "Target Place doesn't exist. " ;
            MASS_base::log(convert.str( ));
        }
        return NULL;
    }
    // all good. go ahead and check target agent id as the remote place exists
    int targetPlaceNextAgentId = *remote_targetAgentId;
    if(showLog){
        convert << "Target Place Next AgentId:: "<< targetPlaceNextAgentId;
        MASS_base::log(convert.str( ));
    }
    if(targetPlaceNextAgentId == -1 || targetPlaceNextAgentId == agentId)
    {
        // migrate
        vector<int> dest;
        dest[0] = absMigX;
        dest[1] = absMigY;
        migrate(dest);
    }
    return NULL;
}

void *SugarAgent::metabolize(void *argument )
{
    void *argMetabolism = new int();
    *(int*)argMetabolism = metabolism;
    
    int sugarConsumption = *(int*)(place->callMethod(SugarPlace::consumeSugarAndAccumulatePollution_,argMetabolism));
    
    sugar += sugarConsumption;
    sugar -= metabolism;
    if( sugar < 0 )
    {
        kill();
    }
    return NULL;
}

void *SugarAgent::showAgent()
{
    ostringstream convert;
    convert << "Current Agent's Id at [" << index[0] << "][" << index[1]
    << "] :: " << agentId << "\n";
    
    MASS_base::log( convert.str( ) );
    return NULL;
}

int* SugarAgent::getAgentFutureCoordinateAndId()
{
    int *agentFutureCoordinateAndId = new int[3];
    agentFutureCoordinateAndId[0] = agentId;
    agentFutureCoordinateAndId[1] = absMigX;
    agentFutureCoordinateAndId[2] = absMigY;
    
    return agentFutureCoordinateAndId;
}

int SugarAgent::getAgentId()
{
    return agentId;
}
