#include "ExchangeHelper.h"
#include "MASS_base.h"  // MASS_base::log()
#include <errno.h>      // errno
#include <sstream>      // ostringstream

Socket *ExchangeHelper::socket;
int *ExchangeHelper::sockets;

void ExchangeHelper::establishConnection( int size, int rank, 
					  vector<string> hosts, int port ) {
  ostringstream convert;

  // create a Socket object
  socket = new Socket( port );
  // create sockets[]
  sockets = new int[size];

  // accept connections from higher ranks
  for ( int i = rank + 1; i < size; i++ ) {
    // for debug
    convert.str( "" );
    convert << "rank[" << rank << "] will accept " << i << "-th connection";
    MASS_base::log( convert.str( ) );

    // accept a new connection
    int sd = socket->getServerSocket( );
    
    // retrieve the client socket ipaddress and port
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof( clientAddr );
    if ( getpeername( sd, (sockaddr *)&clientAddr, &addrLen ) != 0 ) {
      convert.str( "" );
      convert << "getpeername: " << errno;
      MASS_base::log( convert.str( ) );
      exit( -1 );
    }
    char *ipaddr = inet_ntoa( clientAddr.sin_addr );

    // access DNS to verify the client ipaddress
    struct hostent *hptr;
    unsigned int addr = inet_addr( ipaddr );
    if ( ( hptr = 
	   gethostbyaddr( ( unsigned int * )&addr, sizeof( unsigned int ),
			  AF_INET ) ) == NULL ) {
      convert.str( "" );
      convert << "gethostbyaddr error for the client( " << ipaddr
	      << "): " << h_errno;
      MASS_base::log( convert.str( ) );
      exit( -1 );
    }
    convert.str( "" );
    convert << "connection from " << hptr->h_name;
    MASS_base::log( convert.str( ) );

    // idenfity the rank of this connection from h_name
    for ( int j = rank + 1; j < size; j++ ) {
      convert.str( "" );
      convert << "compare with " << hosts[j];
      MASS_base::log( convert.str( ) );

      if ( strncmp( hosts[j].c_str( ), hptr->h_name, hosts[j].size( ) ) == 0 ){
	sockets[j] = sd; // matched and assigned this socket to rank j.
	convert.str( "" );
	convert << "rank" << rank << "] accepted from rank[" 
		<< j << "]:" << hosts[j];
	MASS_base::log( convert.str( ) );
	break;
      }
    }
  }
  
  // sends connection requests to lower ranks
  for ( int i = 0; i < rank; i++ ) {
    sockets[i] =  socket->getClientSocket( hosts[i].c_str( ) );

    // for debug
    convert.str( "" );
    convert << "rank[" << rank << "] has connected to rank[" << i 
	    << "]: " << hosts[i];
    MASS_base::log( convert.str() );
  }
  
}

void ExchangeHelper::sendMessage( int rank, Message *exchangeReq ) {

  ostringstream convert;
  convert << "exchange.sendMessage serialize msg for : " << rank;
  MASS_base::log( convert.str( ) );

  int msg_size = 0;
  char *byte_msg = exchangeReq->serialize( msg_size );

  convert.str( "" );
  convert << "exchange.sendMessage will be sent to rank: " << rank;
  MASS_base::log( convert.str( ) );

  write( sockets[rank], (void *)&msg_size, sizeof( int ) ); // send a msg size
  write( sockets[rank], byte_msg, msg_size );               // send a msg body
  fsync( sockets[rank] );

  convert.str( "" );
  convert << "exchange.sendMessage has been sent to rank: " << rank;
  MASS_base::log( convert.str( ) );
}

Message *ExchangeHelper::receiveMessage( int rank ) {
  int size = -1;
  int nRead = 0;

  ostringstream convert;
  convert << "exchange.receiveMessage will receive from rank: " << rank;
  MASS_base::log( convert.str( ) );

  if ( read( sockets[rank], 
	     (void *)&size, sizeof( int ) ) > 0 ) { // receive a msg size

    convert.str( "" );
    convert << "exchange.receiveMessage received size = " << size
	    << " from rank: " << rank;
    MASS_base::log( convert.str( ) );

    char *buf = new char[size];
    for ( nRead = 0;                                   // receive a msg body
	  ( nRead += read( sockets[rank], buf + nRead, size - nRead ) ) 
	    < size; );
    Message *m = new Message( );
    m->deserialize( buf, size );

  convert.str( "" );
  convert << "exchange.receiveMessage has received from rank: " << rank;
  MASS_base::log( convert.str( ) );

    return m;
  } else {
    convert.str( "" );
    convert << "ExchangeHelper.receiveMessage error from rank[" << rank << "]";
    MASS_base::log( convert.str( ) );
    exit( -1 );
  }
}
