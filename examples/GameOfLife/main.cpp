
#include "MASS.h"
#include "Life.h"
#include "Timer.h"    //Timer
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>
#include <iostream>

using namespace std;

const int iterPerTurn = 9;                            //Partitioned movespaces are 9 squares large
//Used to toggle output
//const bool printOutput = false;
const bool printOutput = true;

Timer timer;

int main(int argc, char *args[]) {

    // check that all arguments are present
    // exit with an error if they are not
    if (argc != 10) {
        cerr << "usage: ./main username password machinefile port nProc nThreads numTurns sizeX sizeY" << endl;
        return -1;
    }

    // get the arguments passed in
    char *arguments[4];
    arguments[0] = args[1]; // username
    arguments[1] = args[2]; // password
    arguments[2] = args[3]; // machinefile
    arguments[3] = args[4]; // port
    int nProc = atoi(args[5]); // number of processes
    int nThr = atoi(args[6]);  // number of threads

    const int numTurns = atoi(args[7]);    //Run this simulation for numTurns
    const int sizeX = atoi(args[8]);
    const int sizeY = atoi(args[9]);
    const double rmSpawn = 0.2;
    const int myPopulation = sizeX * sizeY * rmSpawn * rmSpawn;    //Population{RMsize, RMSpawn}
    static const int totalSize = sizeX * sizeY;                            //Total size of map

    // initialize MASS with the machine information,
    // number of processes, and number of threads
    MASS::init(arguments, nProc, nThr);

    // prepare a message for the places (this is argument below)
    char *msg = (char *) ("hello\0"); // should not be char msg[]

    /*  THIS SECTION OF CODE DEALS ONLY WITH PLACES  */

    // Create the places.
    // Arguments are, in order:
    //    handle, className, boundary_width, argument, argument_size, dim, ...
    Places *life = new Places(1, "Life", 1, msg, 6, 2, sizeX, sizeY);


    // define the destinations, which represent the Places
    // adjacent to a particular place (represented by [0, 0].
    //        [0, 1]                 [ north]
    // [-1, 0][0, 0][1, 0]  == [west][origin][east]
    //        [0,-1]                 [ south]
    // Each X is represent by an array containing its coordinates.
    // Note that you can have an arbritrary number of destinations.  For example,
    // northwest would be [-1,1].
    vector<int *> destinations;
    int north[2] = {0, 1};
    destinations.push_back(north);
    int east[2] = {1, 0};
    destinations.push_back(east);
    int south[2] = {0, -1};
    destinations.push_back(south);
    int west[2] = {-1, 0};
    destinations.push_back(west);
    int northwest[2] = {-1, 1};
    destinations.push_back(northwest);
    int northeast[2] = {1, 1};
    destinations.push_back(northeast);
    int southwest[2] = {-1, -1};
    destinations.push_back(southwest);
    int southeast[2] = {1, -1};
    destinations.push_back(southeast);

    life->callAll(Life::init_);                //initialize life

    timer.start();

    for (int turn = 0; turn < numTurns; turn++) {

        if (printOutput) { cout << "Iteration : " << turn << endl; }

        // set out message to alive or dead, then display the current health status of the life
        life->callAll(Life::displayHealthStatus_);

        life->exchangeBoundary();  // exchange shadow space information

        //exchange information on whether curr life is dead or alive.
        life->callAll(Life::getBoundaryHealthStatus_);

        // need to compute dead or alive based on neighbors
        life->callAll(Life::computeDeadOrAlive_);

        if (printOutput) {
            cout << "********************************************************************************" << endl;
        }
    }

    cout << "Health Status after " << numTurns << endl;
    int *maxTurns = new int;
    *maxTurns = numTurns;

    long elaspedTime_END = timer.lap();
    printf("\nEnd of simulation. Elasped time using MASS framework with %i processes "
           "and %i thread and %i turns :: %ld \n",nProc, nThr, numTurns, elaspedTime_END);

    MASS::finish();
}
