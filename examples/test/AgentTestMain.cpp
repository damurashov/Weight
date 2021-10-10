#include <iostream>
#include <math.h>   // floor
#include "Timer.h"
#include "TestPlace.h"
#include "TestAgent.h"
#include "MASS.h"
#include <stdlib.h> // atoi
#include <stdio.h>  // fprintf
#include <unistd.h>
#include <vector>

using namespace std;

int main(int argc, char * args[]) {
  if (argc != 11) {
    //convert to config file + args later
    cout
        << "usage: ./TestMain username password machinefile port nProc nThr test_type size max_time iterations"
        << endl;
    return -1;
  }
  char *net_args[4];
  net_args[0] = args[1]; // username
  net_args[1] = args[2]; // password
  net_args[2] = args[3]; // machinefile
  net_args[3] = args[4]; // port
  int nProc = atoi(args[5]);
  int nThr = atoi(args[6]);

  int test_type = atoi(args[7]);
  int size = atoi(args[8]);
  int max_time = atoi(args[9]);
  int iterations = atoi(args[10]);

  cerr << "main called with the following arguments:" << endl;
  cerr << "username: " << net_args[0] << endl;
  cerr << "password: " << net_args[1] << endl;
  cerr << "machinefile: " << net_args[2] << endl;
  cerr << "port: " << net_args[3] << endl;
  cerr << "nProc: " << nProc << endl;
  cerr << "nThr: " << nThr << endl;
  cerr << "test_type: " << test_type << endl;
  cerr << "size: " << size << endl;
  cerr << "max_time: " << max_time << endl;
  cerr << "iterations: " << iterations << endl;

  cerr << "Initializing Mass ( MASS::init(...) )..." << endl;
  MASS::init(net_args, nProc, nThr);
  cerr << "Initialization complete." << endl;
  char * msg = "HI";
  Places *elems = new Places(1, "TestPlace", 1, msg, 3, 2, size, size);
  cerr << "completed Places instantiation" << endl;
  elems->callAll(TestPlace::init_, &iterations, sizeof(int));
  cerr << "completed Places init" << endl;
  Agents *agents = new Agents(2, "TestAgent", msg, 3, elems, size * size);
  cerr << "completed Agents instantiation" << endl;
  agents->callAll(TestAgent::init_, &iterations, sizeof(int));
  cerr << "completed Agents init" << endl;
  vector<int*> destinations;
  int north[2] = { 0, 1 };
  destinations.push_back(north);
  int east[2] = { 1, 0 };
  destinations.push_back(east);
  int south[2] = { 0, -1 };
  destinations.push_back(south);
  int west[2] = { -1, 0 };
  destinations.push_back(west);
  Timer time;
  time.start();

  cerr << "Calling TestAgent::report_" << endl;
  int *retvals = (int *) (agents->callAll(TestAgent::report_,
  NULL, 0, sizeof(int) * 2));
  //print return values
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size * 2; j += 2) {
      fprintf(stderr, "%d,%d ", retvals[i * size * 2 + j],
          retvals[i * size + j + 1]);
    }
    cerr << endl;
  }
  cerr << endl;
  cerr << "Completed TestAgent::report_" << endl;

  switch (test_type) {
    case 5: // test best migrate
      fprintf(stderr, "Entering test 5: TestAgent::best_migrate_\n");
      agents->callAll(TestAgent::best_migrate_);
      cerr << "Finished 1st callALL\n";
      agents->manageAll();
      cerr << "Finished manageAll()\n";
      retvals = (int *) (agents->callAll(TestAgent::report_,
      NULL, 0, sizeof(int) * 2));

      cerr << "Finished getting retvals" << endl;
      //print return values
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size * 2; j += 2) {
          fprintf(stderr, "%d,%d ", retvals[i * size * 2 + j],
              retvals[i * size + j + 1]);
        }
        cerr << endl;
      }
      cerr << " Finished Test 5" << endl;
      cerr << endl;
      break;
    case 6: // test random migrate
      fprintf(stderr, "Entering test 6: TestAgent::random_migrate_\n");
      agents->callAll(TestAgent::random_migrate_, &size, sizeof(int));
      agents->manageAll();
      retvals = (int *) (agents->callAll(TestAgent::report_,
      NULL, 0, sizeof(int) * 2));
      //print return values
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size * 2; j += 2) {
          fprintf(stderr, "%d,%d ", retvals[i * size * 2 + j],
              retvals[i * size + j + 1]);
        }
        cerr << endl;
      }
      cerr << endl;
      break;
    case 7: // test worst migrate
      fprintf(stderr, "Entering test 7: TestAgent::worst_migrate_\n");
      int args[2];
      args[0] = size;
      args[1] = nProc;
      agents->callAll(TestAgent::worst_migrate_, &args, sizeof(int) * 2);
      agents->manageAll();
      retvals = (int *) (agents->callAll(TestAgent::report_, NULL, 0,
          sizeof(int) * 2));
      //print return values
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size * 2; j += 2) {
          fprintf(stderr, "%d,%d ", retvals[i * size * 2 + j],
              retvals[i * size + j + 1]);
        }
        cerr << endl;
      }
      cerr << endl;
      break;
    default:
      fprintf(stderr, "Entering default test:\n");
      for (int i = 0; i < max_time; i++) {
        switch (test_type) {
          case 1:	// test for Agents callAll granularity
            fprintf(stderr, "Entering test 1: TestAgent::call_all_\n");
            agents->callAll(TestAgent::call_all_);
            break;
          case 2: //random migrate granularity
            fprintf(stderr, "Entering test 2: TestAgent::random_migrate_\n");
            agents->callAll(TestAgent::random_migrate_, &size, sizeof(int));
            agents->manageAll();
            break;
          case 3: //full migrate granularity
            fprintf(stderr, "Entering test 3: TestAgent::worst_migrate_\n");
            int args[2];
            args[0] = size;
            args[1] = nProc;
            agents->callAll(TestAgent::worst_migrate_, args, sizeof(int) * 2);
            agents->manageAll();
            break;
          case 4:
            fprintf(stderr, "Entering test 4: TestAgent::call_all_return_\n");
            if (i % 4 == 0) {

              double *retvals = (double *) agents->callAll(
                  TestAgent::call_all_return_,
                  NULL, 0, sizeof(double));
              //print return values
              for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                  fprintf(stderr, "%4.2f ", retvals[i * size + j]);
                }
                cerr << endl;
              }
              cerr << endl;
            } else {
              agents->callAll(TestAgent::call_all_);
            }

            break;
          default:
            cerr << "Error! default switch case" << endl;
            break;
        }
      }
  }
  cerr << "Elapsed time = " << endl << time.lap() << endl;
  return 0;

}
