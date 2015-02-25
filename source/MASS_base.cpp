#include <sstream>      // ostringstream
#include "MASS_base.h"

//Used to toggle output for MASS_base
//const bool printOutput = false;
const bool printOutput = true;

// Allocate static space
/**
 * Port number to establish connections for interprocess communication
 */
int MASS_base::MASS_PORT = 0;
/**
 * Track whether or not this simulation has initialized all Threads yet
 */
bool MASS_base::INITIALIZED = false;
/**
 * C String of current working directory (char array)
 */
char MASS_base::CUR_DIR[CUR_SIZE];
/**
 * String of host running this instance of MASS
 */
string MASS_base::hostName;
/**
 * Numeric representation of this node's rank (master == 0)
 */
int MASS_base::myPid = 0;
/**
 * Number of processes running simulation
 */
int MASS_base::systemSize = 0;
/**
 * Reference to a simple output stream logger
 */
ofstream MASS_base::logger;
/**
 * Collection of host names concurrently working on this simulation
 */
vector<string> MASS_base::hosts;
/**
 * Collection of threads running this MASS instance
 */
vector<pthread_t> MASS_base::threads;
/**
 * Lock to prevent competing threads on a machine from interleaving log content
 */
pthread_mutex_t MASS_base::log_lock;
/**
 * Lock to prevent competing threads from entering critical sections out of turn
 */
pthread_mutex_t MASS_base::request_lock;
/**
 * Associative container that stores all Places collections for a given handle
 */
map<int, Places_base*> MASS_base::placesMap;
/**
 * Associative container that stores all Agents collections for a given handle
 */
map<int, Agents_base*> MASS_base::agentsMap;
/**
 * Associative container that stores all DllClass references for a given handle
 */
map<int, DllClass*> MASS_base::dllMap;
/**
 * Collection of pointers to containers of RemoteExchangeRequest pointers - you
 * can pretty much think of this as a massive queue of all remote exchange
 * requests
 */
vector<vector<RemoteExchangeRequest*>*> MASS_base::remoteRequests;
/**
 * Collection of pointers to containers of AgentMigrationRequest pointers - you
 * can pretty much think of this as a massive queue of all Agent migration
 * requests
 */
vector<vector<AgentMigrationRequest*>*> MASS_base::migrationRequests;
/**
 * Should store the number of requests in queue, but doesn't appear to be fully
 * implemented yet (value is set to zero and never changed)
 */
int MASS_base::requestCounter;
/**
 * Collection of Places_base Object references (pointers) that should refer to
 * the current set of Places (usually retrieved/set for a given handle)
 */
Places_base *MASS_base::currentPlaces;
/**
 * Collection of Places_base Object references (pointers) that should refer to
 * an alternate set of Places in the simulation (usually a 'future' set of
 * Places after an exchangeAll has occurred)
 */
Places_base *MASS_base::destinationPlaces;
/**
 * Collection of Agents_base Object references (pointers) that should refer to
 * the current set of Agents (usually retrieved/set for a given handle)
 */
Agents_base *MASS_base::currentAgents;
/**
 * Index (int) of the function ID currently being called on a user-defined
 * Agent or Place class
 */
int MASS_base::currentFunctionId;
/**
 * Reference to (pointer) the current argument being processed for an Agent or
 * Place method request
 */
void *MASS_base::currentArgument;
/**
 * Current size of the argument being processed for an Agent or Place method
 * request
 */
int MASS_base::currentArgSize;
/**
 * Current size of the return value being stored for an Agent or Place method
 * call
 */
int MASS_base::currentRetSize;
/**
 * Reference to (pointer) the current return values stored as a result of an
 * Agent or Place method call
 */
char *MASS_base::currentReturns;
/**
 * Collection of pointers to int pointers that refer to the global array index
 * of current destinations for an exchange all request
 */
vector<int*> *MASS_base::currentDestinations;
/**
 * This is the type of the current message being processed. More information on
 * available types can be found in Message.h (Message::ACTION_TYPE)
 */
Message::ACTION_TYPE MASS_base::currentMsgType;
/**
 * Reference to the helper class that helps negotiate socket communication
 */
ExchangeHelper MASS_base::exchange;

/**
 * This method initializes core (base) MASS Object data attributes.
 * 
 * @param name  the name of the host running this instance of MASS
 * @param myPid the numeric rank of this process ID (master == 0)
 * @param nProc the number of processes to run simulation on
 * @param port  the port to use for group communication
 */
void MASS_base::initMASS_base( const char *name, int myPid, int nProc,
    int port ) {
  // Initialize constants
  MASS_base::hostName = name;
  MASS_base::myPid = myPid;
  MASS_base::systemSize = nProc;
  MASS_base::MASS_PORT = port;
  MASS_base::currentPlaces = NULL;
  MASS_base::currentAgents = NULL;
  MASS_base::requestCounter = 0;
  pthread_mutex_init( &MASS_base::log_lock, NULL );
  pthread_mutex_init( &MASS_base::request_lock, NULL );

  // Get the current working directory
  bzero( MASS_base::CUR_DIR, CUR_SIZE );
  getcwd( MASS_base::CUR_DIR, CUR_SIZE );
  if ( strlen( MASS_base::CUR_DIR ) == 0 ) {
    if ( printOutput == true )
      cerr << "getcwd failed" << endl;
    exit( -1 );
  }
}

/**
 * This method initializes the individual threads for a given MASS instance.
 * 
 * @param nThr  number of Threads to spin up for this instance of MASS
 * @return      true if Threads initialized successfully, else false
 */
bool MASS_base::initializeThreads( int nThr ) {
  if ( INITIALIZED ) {
    if ( printOutput == true )
      cerr << "Error: the MASS.init is already initializecd" << endl;
    return false;
  }

  int cores = ( nThr <= 0 ) ? getCores( ) : nThr;

  // all pthread_t structures
  threads.reserve( cores );
  threads.push_back( pthread_self( ) ); // the main thread id

  pthread_t thread_ref;                 // a temporary thread reference

  // initialize Mthread's static variables
  Mthread::init( );

  // now launch child threads
  Mthread::threadCreated = 0;
  for ( int i = 1; i < cores; i++ ) {
    if ( pthread_create( &thread_ref, NULL, Mthread::run, &i ) != 0 ) {
      log( "pthread_create: error in MASS_base::initializeThreads" );
      exit( -1 );
    }
    threads.push_back( thread_ref );
    while ( Mthread::threadCreated != i )
      ; // busy wait
  }

  ostringstream convert;
  if ( printOutput == true ) {
    convert << "Initialized threads - # " << cores;
    log( convert.str( ) );
  }

  INITIALIZED = true;
  return true;
}

/**
 * Logs a message to Mass's internal logs. If this instance of MASS represents
 * the master node (myPid == 0), then logs are printed directly to the cerr
 * output. Otherwise, all slave nodes print out messages to the following file:
 *
 * <CUR_DIR>/MASS_logs/PID_<myPid>_<hostname>result.txt
 *
 * @param msg String of message to print to output log
 */
void MASS_base::log( string msg ) {
  pthread_mutex_lock( &log_lock );
  if ( myPid == 0 ) {
    // The master node directly prints out the message to standard error.
    cerr << msg << endl;
  } else {
    // All the slave nodes prints out the message to CUR_DIR/MASS_logs/.
    if ( !logger.is_open( ) ) {
      // if not open, then open a logger.
      ostringstream convert;
      convert << MASS_LOGS << "/PID_" << myPid << "_" << hostName
          << "result.txt";
      string filename = convert.str( );
      logger.open( filename.c_str( ), ofstream::out );
    }
    logger << msg << endl;
  }
  pthread_mutex_unlock( &log_lock );
}

/**
 * Sets the hosts that MASS is using and establishes SSH communication for the
 * group to use.
 *
 * @param host_args list of host names that MASS should run on
 */
void MASS_base::setHosts( vector<string> host_args ) {
  if ( !hosts.empty( ) ) {
    // already initialized
    return;
  }

  // register all hosts including myself
  for ( int i = 0; i < int( host_args.size( ) ); i++ ) {
    if ( printOutput == true )
      log( host_args[i] );
    hosts.push_back( host_args[i] );
  }

  // instantiate remoteRequests: vector< vector<RemoteExchangeReques*> >
  // as well as migrationRequests for the purpose of agent migration.
  for ( int i = 0; i < systemSize; i++ ) {
    remoteRequests.push_back( new vector<RemoteExchangeRequest*> );
    migrationRequests.push_back( new vector<AgentMigrationRequest*> );
  }

  // establish inter-MASS connection
  exchange.establishConnection( systemSize, myPid, hosts, MASS_PORT );
}

/**
 * Logs the hosts MASS is using.
 */
void MASS_base::showHosts( ) {
  ostringstream convert;
  if ( printOutput == true ) {
    convert << "hosts....." << endl;
    for ( int i = 0; i < int( hosts.size( ) ); i++ ) {
      convert << "rank[" << i << "] = " << hosts[i] << endl;
    }
    MASS_base::log( convert.str( ) );
  }
}

/**
 * Returns the number of cores currently available on the machine. If -1 is
 * returned, then the option is not supported on this platform. If zero is
 * returned, then it means that the relevant functions and headers exist, but
 * one has to ask at run time what degree of support is available. Else, will
 * return the number of available cores.
 *
 * @return  numeric representation of the number of cores available on this
 *          machine (will not count configured cores that are
 *          offline/unavailable at runtime)
 */
int MASS_base::getCores( ) {
  return sysconf( _SC_NPROCESSORS_ONLN );
}
