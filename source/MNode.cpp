/*
 MASS C++ Software License
 © 2014-2015 University of Washington
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
 © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "MNode.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

//Used to toggle output for MNode
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

using namespace std;

/**
* 
*/
void MNode::closeMainConnection( ) {
}

/**
 * 
 * @param m
 */
void MNode::sendMessage( Message *m ) {
  int size;
  char *buf = m->serialize( size );
  if(printOutput == true)
      cerr << "size = " << size << endl;
  if ( write( sd, (char *)&size, sizeof( int ) ) > 0 ) {
    if ( write( sd, buf, size ) > 0 ) {
      return;
    }
  }
  MASS_base::log( "sendMessage error" );
  exit( -1 );
}

/**
 * 
 * @return 
 */
Message *MNode::receiveMessage( ) {
  int size = -1;
  int nRead = 0;
  if ( ( nRead = read( sd, (char *)&size, sizeof( int ) ) ) > 0 ) {
    if(printOutput == true)
        cerr << "nRead = " << nRead << ", size = " << size << endl;
    char *buf = new char[size];
    for ( nRead = 0; 
	  ( nRead += read( sd, buf + nRead, size - nRead) ) < size; );
    if(printOutput == true)
        cerr << "nRead = " << nRead << endl;
    if ( size > 4 ){
      if(printOutput == true)
          cerr << "*(int *)(buf + 4) = " << *(int *)(buf + 4) << endl;
    }
    Message *m = new Message( );
    m->deserialize( buf, size );
    return m;
  }
  else if ( nRead <= 0 ) {
    ostringstream convert;
    convert << "receivMessage error from rank[" << pid << "] at " 
	    << hostName;
    MASS_base::log( convert.str( ) );
    exit( -1 );
  }
  return NULL;
void MNode::closeMainConnection() {
	close(sd);
}

Ssh2Connection *MNode::getConnection() {
	return connection;
}

