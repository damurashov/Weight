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
  vector<string> hosts;  // all hosts participated in computation

  void sendAck( );
  void sendAck( int localPopulation );
  void sendReturnValues( void *argument, int places_size, int return_size );
  void sendMessage( Message *msg );
  Message *receiveMessage( );
};

#endif
