#ifndef MPROCESS_H
#define MPROCESS_H

#include <vector>
#include <string>
#include <map>
#include "Message.h"

using namespace std;

class MProcess {
 public:
  MProcess( char *hostName, int myPid, int nProc, int nThr, int port );
  void start( );
 private:
  string *hostName; // my local host name 
  int myPid;        // my pid or rank
  int nProc;        // # processes
  int nThr;         // # threads
  int port;         // TCP port
  int sd;           // bare TCP socket to the master
  vector<string> hosts;  // all hosts participated in computation

  void sendAck( );
  void sendAck( int localPopulation );
  void sendReturnValues( void *argument, int nPlaces, int return_size );
  void sendReturnValues( void *argument, int nAgents, int return_size, 
			 int localPopulation );
  void sendMessage( Message *msg );
  Message *receiveMessage( );
};

#endif
