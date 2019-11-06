//
//  SugarPlace.h
//  Sugarscape using MASS
//
//  Created by Sourish Chatterjee on 3/14/15.
//  

#ifndef __Sugarscape__SugarPlace__
#define __Sugarscape__SugarPlace__


#include "Place.h"

using namespace std;

 class SugarPlace : public Place {
     
 public:
     
     static const int init_= 0;
     static const int incSugarAndPollution_ = 1;     
     static const int updatePollutionWithAverage_ = 2;
     static const int showSugar_= 3;
     static const int showPollution_= 4;
     static const int setOutAgentCountAndSugPolRatio_ = 6;
     static const int setOutNextAgentId_ = 7;
     static const int setOutPollution_ = 8;
     static const int prePlaceExchangeAllSetMigData_ = 9;
     static const int postMigrationReset_ = 11;
     static const int avePollutions_ = 12;
     static const int getRemotePlaceInfo_ = 13;
     static const int setAgentFutureCoordinateAndId_ = 14;
     static const int getRemotePlaceFutureAgentId_ = 15;
     static const int consumeSugarAndAccumulatePollution_ = 16;
     
     SugarPlace(void *argument) : Place( argument ) { };
     
     virtual void *callMethod( int functionId, void *argument )
     {
         switch( functionId )
         {
             case init_:return init(argument);
             case incSugarAndPollution_: return incSugarAndPollution( );
             case avePollutions_: return avePollutions( );
             case updatePollutionWithAverage_: return updatePollutionWithAverage( );
             case showSugar_: return showSugar( );
             case showPollution_: return showPollution( );
             case setOutAgentCountAndSugPolRatio_: return setOutAgentCountAndSugPolRatio( );
             case setOutPollution_: return setOutPollution( );
             case prePlaceExchangeAllSetMigData_:return prePlaceExchangeAllSetMigData();
             case setOutNextAgentId_:return setOutNextAgentId(argument);
             case postMigrationReset_:return postMigrationReset();
             case getRemotePlaceInfo_:return getRemotePlaceInfo(argument);
             case setAgentFutureCoordinateAndId_:return setAgentFutureCoordinateAndId(argument);
             case getRemotePlaceFutureAgentId_:return getRemotePlaceFutureAgentId(argument);
             case consumeSugarAndAccumulatePollution_:return consumeSugarAndAccumulatePollution(argument);
         }
         return NULL;
     };
     
     bool checkIdMarkable(int id1);
     double getSugarPollutionRatio();
     
     
   private:
     int curSugar;          // the current sugar amount
     int maxSugar;          // the max capability of holding sugar
     double pollution;    // the current pollution
     double avePollution; // averaging four neighbors' pollution
     int nextAgentId;      // the next agent to come here
     int currAgtFutureCoord[2];
     int migratingAgentId;
     
     int initSugarAmount(int mtPeakX, int mtPeakY, int maxMtSug );
     
     void *init(void *argument);
     void *incSugarAndPollution();
     void *avePollutions();
     void *updatePollutionWithAverage( );
     void *showSugar();
     void *showPollution();
     void *setOutAgentCountAndSugPolRatio();
     void *setOutNextAgentId(void *argument);
     void *setOutPollution();
     void *prePlaceExchangeAllSetMigData();     
     void *postMigrationReset();
     
     void *getRemotePlaceInfo(void *argument);
     void *setAgentFutureCoordinateAndId(void *argument);
     void *getRemotePlaceFutureAgentId(void *argument);
     void *consumeSugarAndAccumulatePollution(void *argument); // input: metabolism
     
 };

#endif /* defined(__Sugarscape__SugarPlace__) */
