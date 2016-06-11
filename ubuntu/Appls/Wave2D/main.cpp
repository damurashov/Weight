#include "MASS.h"
#include "Wave2DMass.h"
#include <stdlib.h> // atoi
#include <unistd.h>
#include <vector>
#include "Timer.h"

//Used to toggle output for Main
const bool printOutput = false;
//const bool printOutput = true;

int main( int argc, char *args[] ) {

  if ( argc != 9 ) {
    cerr << "usage: ./main username password machinefile port nProc nThr"<< endl;
    return -1;
  }

  char *arguments[4];
  arguments[0] = args[1]; // username
  arguments[1] = args[2]; // password
  arguments[2] = args[3]; // machinefile
  arguments[3] = args[4]; // port
  int nProc = atoi( args[5] ); //# of process
  int nThr = atoi( args[6] );  //# of thread
  int size = atoi( args[7] );  //# of array size
  int maxTime = atoi( args[8] );  //# of time
  Timer timer;  
  MASS::init( arguments, nProc, nThr );
  timer.start();
  //cout<<"before create places"<<endl;
  Places *wave2d = new Places( 1, "Wave2DMass", NULL, 0, 2, size, size );
  //cout<<"after create places"<<endl;
  wave2d->callAll( Wave2DMass::init_);
  //cout<<"after create init"<<endl;
  vector<int*> neighbors;
  int north[2] = {0, -1};  neighbors.push_back( north );
  int east[2]  = {1, 0};  neighbors.push_back( east );
  int south[2] = {0, 1}; neighbors.push_back( south );
  int west[2]  = {-1, 0}; neighbors.push_back( west );
  // now go into a cyclic simulation
  for ( int time = 0; time < maxTime; time++ ) {
    //cout<<"simulating..."<<time<<endl;
	wave2d->callAll( Wave2DMass::computeWave_, (void *)&time, sizeof(int) );
	//cout<<"simulating...after callAll "<<time<<endl;
        wave2d->exchangeBoundary( );
	//cout<<"simulating... after exchangeAll"<<time<<endl;
  }

   if(printOutput){
    double *all = (double *)wave2d->callAll(Wave2DMass::collectWave_, NULL, 0, sizeof(double));
    for(int i=0; i<size; i++){
      for(int j=0; j<size; j++)
	cout << *(all+i*size+j)<<" ";
      cout<<endl;
    }
  }
  cout << "elapsed time = " << timer.lap( ) << endl;
  MASS::finish( );

}
