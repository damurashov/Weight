#include "Ssh2Connection.h"

int Ssh2Connection::read( char *buf, int size ) {
  return ( int )libssh2_channel_read( channel, buf, size );
}

int Ssh2Connection::write( char *buf, int size ) {
  return ( int )libssh2_channel_write( channel, buf, size );
}

