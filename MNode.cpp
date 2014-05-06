#include "MNode.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

using namespace std;

void MNode::closeMainConnection( ) {
}

void MNode::sendMessage( Message *m ) {
  int size;
  char *buf = m->serialize( size );
  cerr << "size = " << size << endl;
  if ( connection->write( (char *)&size, sizeof( int ) ) > 0 ) {
    if ( connection->write( buf, size ) > 0 ) {
      return;
    }
  }
  MASS_base::log( "sendMessage error" );
  exit( -1 );
}

Message *MNode::receiveMessage( ) {
  int size = -1;
  int nRead = 0;
  if ( ( nRead = connection->read( (char *)&size, sizeof( int ) ) ) > 0 ) {
    cerr << "nRead = " << nRead << ", size = " << size << endl;
    char *buf = new char[size];
    for ( nRead = 0; 
	  ( nRead += connection->read(buf + nRead, size - nRead) ) < size; );
    cerr << "nRead = " << nRead << endl;
    if ( size > 4 ) cerr << "*(int *)(buf + 4) = " << *(int *)(buf + 4) << endl;
    Message *m = new Message( );
    m->deserialize( buf, size );
    return m;
  }
  else if ( nRead < 0 ) {
    ostringstream convert;
    convert << "receivMessage error from rank[" << pid << "] at " 
	    << hostName;
    MASS_base::log( convert.str( ) );
    exit( -1 );
  }
  return NULL;
}

