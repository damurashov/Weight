//
//  SugarPlace.cpp
//  Sugarscape using MASS
//
//  Created by Sourish Chatterjee on 3/14/15.
//  
//

#include "SugarPlace.h"
#include "MASS_base.h"
#include <sstream>     // ostringstream
#include <math.h>

const bool printOutput = true;


extern "C" Place* instantiate( void *argument ) {
    return new SugarPlace( argument );
}

extern "C" void destroy( Place *object ) {
    delete object;
}

// START:: MASS framework compatible methods

void *SugarPlace::init(void *argument)
{
    int simSize = size[0];
    
    pollution = 0.0;    // the current pollution
    avePollution = 0.0; // averaging four neighbors' pollution
    nextAgentId = -1;      // the next agent to come here
    migratingAgentId = -1; // no agent at the beginning
    int mtCoord[2];
    mtCoord[0] = simSize/3;
    mtCoord[1] = simSize - simSize/3 - 1;
    
    int mt1 = initSugarAmount(mtCoord[0],mtCoord[1],*(int*)argument);
    int mt2 = initSugarAmount(mtCoord[1],mtCoord[0],*(int*)argument);
    
    curSugar = mt1 > mt2 ? mt1 : mt2;
    maxSugar = mt1 > mt2 ? mt1 : mt2;;
    
    return NULL;
    
}

int SugarPlace::initSugarAmount(int mtPeakX, int mtPeakY, int maxMtSug )
{
    int x_coord = index[0];
    int y_coord = index[1];
    
    double distance = sqrt(( mtPeakX - x_coord ) * ( mtPeakX - x_coord ) + (mtPeakY - y_coord) * (mtPeakY - y_coord));
    
    //printf("this.X: %i, this.Y, %i, X: %i, Y: %i \n",x,y,x1,y1);
    //printf("Distance:: %f\n", distance);
    // radius is assumed to be simSize/2.
    int r = size[0]/2;
    if ( distance < r )
    {
        // '+ 0.5' for rounding a value.
        return ( int )( maxMtSug + 0.5 - maxMtSug / ( double )r * distance );
    }
    else
        return 0;
}


void *SugarPlace::setOutAgentCountAndSugPolRatio()
{
    outMessage_size = sizeof( int[2] );
    int outMessages[2];
    outMessages[0] = agents.size();
    outMessages[1] = 0;
    
    if(getSugarPollutionRatio() > 0.0)
    {
        outMessages[1] = 1; // permissable to migrate
    }
    
    outMessage = outMessages; // assigning outMessages
    
    return NULL;
}


void *SugarPlace::setOutNextAgentId(void *argument)
{
    // outMessage already set by caller places. This method is used as exchangeall strategy
    if(agents.size() == 0)
    {
        int *outMsg = (int*)argument;
        int agentId = outMsg[0];
        if(agentId != -99 ) // some mig data is there to look at. It might still not be relevant to this place
        {
            int targetX = outMsg[1];
            int targetY = outMsg[2];
            
            if(index[0]==targetX && index[1]==targetY)
            {
                if(nextAgentId < agentId)
                {
                    nextAgentId = agentId;
                }
            }
        }
    }
    
    return NULL;
    
}

void *SugarPlace::postMigrationReset()
{
    nextAgentId = -1;
    migratingAgentId = -1;
    currAgtFutureCoord[0] = -1;
    currAgtFutureCoord[1] = -1;
    
    return NULL;
}

void *SugarPlace::prePlaceExchangeAllSetMigData()
{
    outMessage_size = sizeof( int[3] );
    int outMigData[3];
    outMigData[0] = -99; // an Invalid agent
    
    if(agents.size() > 0)
    {
        if(currAgtFutureCoord[0] != -1 && currAgtFutureCoord[1] != -1 )
        {
            outMigData[0] = migratingAgentId;
            outMigData[1] = currAgtFutureCoord[0];
            outMigData[2] = currAgtFutureCoord[1];
            
        }
    }
    outMessage = outMigData;
    return NULL;
    
}

void *SugarPlace::setOutPollution()
{
    outMessage_size = sizeof( double );
    outMessage = new double( );
    *(double *)outMessage = pollution;
    return NULL;
}

void *SugarPlace::avePollutions()
{
    double shadow[4];
    int top[2] = {0, 1};
    int right[2]  = {1, 0};
    int bottom[2] = {0, -1};
    int left[2]  = {-1, 0};
    
    double *ptr = (double *)getOutMessage( 1, top );
    shadow[0] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (double *)getOutMessage( 1, right );
    shadow[1] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (double *)getOutMessage( 1, bottom );
    shadow[2] = ( ptr == NULL ) ? 0 : *ptr;
    ptr = (double *)getOutMessage( 1, left );
    shadow[3] = ( ptr == NULL ) ? 0 : *ptr;
    
   
    // avePollution is a private member of  place
    avePollution = ( shadow[0] + shadow[1] + shadow[2] + shadow[3] ) / 4.0;
    
    return NULL;
}

void *SugarPlace::setAgentFutureCoordinateAndId(void *argument)
{
    int *args = (int*)argument;
    migratingAgentId  = args[0];
    currAgtFutureCoord[0] = args[1];
    currAgtFutureCoord[1] = args[2];
    
    return NULL;
}

void *SugarPlace::getRemotePlaceFutureAgentId(void *argument)
{
    int *dests =(int*)argument;
    int *remote_val = (int*)getOutMessage(1,dests);
    if(remote_val == NULL)
    {
        return NULL;
    }
    int r_agentId = *remote_val;
    int *ret_val = new int();
    *(int*)ret_val = r_agentId;
   
    return ret_val;
}



void *SugarPlace::incSugarAndPollution( )
{
    if ( curSugar < maxSugar )
    {
        curSugar++;
    }
    pollution += 1.0;
    return NULL;
}

void *SugarPlace::updatePollutionWithAverage( )
{
    pollution = avePollution;
    avePollution = 0.0;
    return NULL;
}


void *SugarPlace::showSugar( )
{
    ostringstream convert;
    convert << "Current Sugar at [" << index[0] << "][" << index[1]
    << "] :: " << curSugar << "\n";
    
    MASS_base::log( convert.str( ) );
    
    return NULL;
}

void *SugarPlace::showPollution()
{
    ostringstream convert;
    convert << "Current Pollution at [" << index[0] << "][" << index[1]
    << "] :: " << pollution << "\n";
    
    MASS_base::log( convert.str( ) );
    return NULL;
}

void *SugarPlace::getRemotePlaceInfo(void* argument)
{
    int *dests = (int*)argument;
    int *remotePlaceInfo = (int*)getOutMessage(1,dests);
    
    void *ret_val = remotePlaceInfo;
    return ret_val;
}

void *SugarPlace::consumeSugarAndAccumulatePollution( void *argument)
{
    int metabolism = *(int*)argument;
    int sugar = curSugar;
    curSugar = 0;
    pollution += metabolism;
    
    void *ret_val = new int();
    *(int*)ret_val = sugar;
    return ret_val;
}

// END:: MASS framework compatible methods



// START:: Other methods used by inter/intra class object(s)

double SugarPlace::getSugarPollutionRatio()
{
    return ( curSugar / ( 1.0 + pollution ) );
}

bool SugarPlace::checkIdMarkable( int id1)
{
    return ( nextAgentId < id1 );
}

// END:: Other methods used by inter/intra class object(s)



