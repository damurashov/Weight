#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

int main( int argc, char **argv ) {
  string cmd = "touch HELLO!";
  cout << cmd << endl;
  system( cmd.c_str( ) );
  return 0;
}
