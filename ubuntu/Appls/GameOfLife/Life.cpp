
#include "Life.h"
#include "MASS_base.h"
#include <stdlib.h>  //rand
#include <sstream>     // ostringstream
#include <fstream>

using namespace std;

//Used to toggle output for Wave2d
//const bool printOutput = false;
const bool printOutput = true;

extern "C" Place *instantiate(void *argument) {
    return new Life(argument);
}

extern "C" void destroy(Place *object) {
    delete object;
}


/**
 * Initializes a Life object.
 */
void *Life::init(void *argument) {

    //Define cardinals
    int north[2] = {0, 1};
    cardinals.push_back(north);
    int east[2] = {1, 0};
    cardinals.push_back(east);
    int south[2] = {0, -1};
    cardinals.push_back(south);
    int west[2] = {-1, 0};
    cardinals.push_back(west);
    int northwest[2] = {-1, 1};
    cardinals.push_back(northwest);
    int northeast[2] = {1, 1};
    cardinals.push_back(northeast);
    int southwest[2] = {-1, -1};
    cardinals.push_back(southwest);
    int southeast[2] = {1, -1};
    cardinals.push_back(southeast);

    for (int i = 0; i < NUM_NEIGHBORS; i++)
        neighborHealthStatus[i] = -1;

    //1 alive, 0 = dead
    alive = rand() % 2;

    outMessage = NULL;
    outMessage_size = 0;

    return NULL;
}

const int Life::neighbor[8][2] = {{0,  1},
                                  {1,  0},
                                  {0,  -1},
                                  {-1, 0},
                                  {-1, 1},
                                  {1,  1},
                                  {-1, -1},
                                  {1,  -1}};


/** (void)
*	Record inmessage data into neighborHealthStatus
*	@pre:	Inmessage must contain integers
*/
/*
Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
Any live cell with two or three live neighbours lives on to the next generation.
Any live cell with more than three live neighbours dies, as if by overpopulation.
Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/
void *Life::computeDeadOrAlive() {
    int totalAlive = 0;
    for (int i = 0; i < (int) cardinals.size(); i++) {
        totalAlive += neighborHealthStatus[i];
    }

    // if alive, continue living if you have 2 or 3 neighbors; otherwise, die
    if (alive) {

        if (totalAlive == 2 || totalAlive == 3) {
            alive = 1;
        } else {
            alive = 0;
        }
    }

        // if dead, only become alive if total alive neighbors is 3 (reproduction)
    else {

        if (totalAlive == 3) {
            alive = 1;
        }
    }

    if (printOutput) {
//        //Debug logging: Expects neighbor populations
//        ostringstream convert;
//
//        //THIS IS FOR CONSOLE OUTPUT (removed for graphical purposes) comment out for graphical output
//        convert << "recordNeighborHealthStatus:  Life[" << index[0] << "][" << index[1]
//                << "]'s next generation position = " << alive << ", north = " << neighborHealthStatus[0] << ", east = "
//                << neighborHealthStatus[1] << ", south = " << neighborHealthStatus[2] << ", west = "
//                << neighborHealthStatus[3] << ", northwest = " << neighborHealthStatus[4] << ", northeast = "
//                << neighborHealthStatus[5] << ", southwest = " << neighborHealthStatus[6] << ", southeast = "
//                << neighborHealthStatus[7];
//
//        MASS_base::log(convert.str());
    }


    return NULL;
}

/** (void)
*	Set the outmessage as health of this life
*/
void *Life::displayHealthStatus() {
    outMessage_size = sizeof(int);
    outMessage = new int();
    *(int *) outMessage = (int) alive;

    if (printOutput) {
        //Debug logging: Expected 400 or nxn number of lives have a single agent on them
        ostringstream convert;

        //this is for standard console output

//        convert << index[0] << " " << index[1] << " and total size = " << size[0] << " , " << size[1];

        // first Life in row
        if (index[1] == 0) {
            convert << "\n";
        }

        convert << *((int *) outMessage) << " ";

        // if last Life on simulation space
        if (index[0] == (size[0]-1) && index[1] == (size[1]-1)) {
            convert << "\n******************************************************************\n";
        }

//        convert << "displayHealthAsOut:  Life[" << index[0] << "][" << index[1] << "] has the health of "
//                << *((int *) outMessage);

        //this is for graphical output
        //convert << *((int *)outMessage);
        MASS_base::log(convert.str());
    }

    return NULL;
}

/** (void)																#Visual logging confirmed 160710
*	Record the outmessage of neighbors as healthstatus
*	pre: neighborHealthStatus is initialized and contains 4 -1s
*/
void *Life::getBoundaryHealthStatus() {
    //Record neighbor population as before
    int North[2] = {0, 1};
    int East[2] = {1, 0};
    int South[2] = {0, -1};
    int West[2] = {-1, 0};
    int northwest[2] = {-1, 1};
    int northeast[2] = {1, 1};
    int southwest[2] = {-1, -1};
    int southeast[2] = {1, -1};

    int *ptr = (int *) getOutMessage(1, North);
    neighborHealthStatus[0] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, East);
    neighborHealthStatus[1] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, South);
    neighborHealthStatus[2] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, West);
    neighborHealthStatus[3] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, northwest);
    neighborHealthStatus[4] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, northeast);
    neighborHealthStatus[5] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, southwest);
    neighborHealthStatus[6] = (ptr == NULL) ? 0 : *ptr;
    ptr = (int *) getOutMessage(1, southeast);
    neighborHealthStatus[7] = (ptr == NULL) ? 0 : *ptr;


    return NULL;
}

