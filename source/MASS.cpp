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
 The following acknowledgment shall be used where appropriate in publications,
 presentations, etc.: © 2014-2015 University of Washington. MASS was developed
 by Computing and Software Systems at University of Washington Bothell. THE
 SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "MASS.h"

// Used to toggle output for MASS.cpp
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

using namespace std;

// Allocate static space
/**
 * Reference to Utilities class that MASS can use to manage SSH connection and
 * launch remote processes
 */
Utilities MASS::util;
vector<MNode *> MASS::mNodes;

/**
 * Involves nProc processes in the same computation and has each
 * process spawn nThr threads.
 *
 * @param args  pointer to the first element in an array of arguments sent to
 *              MASS. The actual values of these elements correspond to (in
 *              order): username, password, machineFilePath, and port
 * @param nProc the number of processes to run simulation on (not including
 *              master process)
 * @param nThr  number of Threads to spin up for this instance of MASS
 */
void MASS::init(char *args[], int nProc, int nThr) {
    vector<string> hosts;

    // variable assignment
    char *username = args[0];
    char *password = args[1];
    char *machineFilePath = args[2];
    int port = atoi(args[3]);

    // Load any *.so dynamic linking files
    // TODO

    // check if nProc <= 1. If so, don't read a machine file
    if ( nProc > 1 ) {
      // Read a given machine file
      ifstream machinefile(machineFilePath, std::ifstream::in);
      if (machinefile.is_open()) {
	// read only nProc - 1.
	int realnProc = 0; // there may be a discrepancy b/w
	                   // nProc and IP names in the machine file
	for ( realnProc = 1; realnProc < nProc; realnProc++ ) {
	  if ( machinefile.good() ) {
	    string machineName;
	    getline(machinefile, machineName);
	    if (machineName.size() > 0)  // skip a null string at the end
	      hosts.push_back(machineName);
	  }
	}
	nProc = realnProc; // if #IP names < nProc - 1
        machinefile.close();
      } else {
        cerr << "machine file: " << machineFilePath << " could not open."
             << endl;
        exit(-1);
      }
    }
    else {
      nProc = 1;
    }
    // For debugging
    if (printOutput == true) {
        for (int i = 0; i < int(hosts.size()); i++)
            cerr << "rank " << (i + 1) << ": " << hosts[i] << endl;
    }

    // Now systemSize counts the actual number of cluster nodes
    systemSize = nProc;

    // Initialize MASS_base.constants and identify the CWD.
    initMASS_base("localhost", 0, nProc, port);

    // For debugging
    cerr << "CUR_DIR = " << CUR_DIR << endl;

    // Launch remote processes
    int pid = 1;  // a slave process id

    for (int i = 0; i < int(hosts.size()); i++, pid++) {
        // retrieve each canonical remote machine name
        string currHostName = hosts[i];
        struct hostent *host = gethostbyname(currHostName.c_str());
        if (host == NULL) {
            cerr << "wrong host name: " << currHostName << endl;
            exit(-1);
        }
        currHostName = host->h_name;

        // For debugging
        if (printOutput == true)
            cerr << "currHostName = " << currHostName << endl;

        // Establish an ssh2 channel to a given port
        Ssh2Connection *ssh2connection = util.establishConnection(
            currHostName.c_str(), LIBSSH2_PORT, username, password);

        // Start a remote process
        string command = CUR_DIR;  // the absolute path to the command
        command += "/mprocess ";   // the command
        command += CUR_DIR;
        command += " ";  // 1st arg: current working dir
        command += currHostName;
        command += " ";  // 2nd arg: hostName
        ostringstream convert;

        convert << pid << " "    // 3rd arg: pid
                << nProc << " "  // 4th arg: #processes
                << nThr << " "   // 5th arg: #threads
                << MASS_PORT;    // 6th arg: MASS_PORT
        command += convert.str();

        if (ssh2connection != NULL) {
            if (!util.launchRemoteProcess(ssh2connection, command.c_str())) {
                // connection failure
                util.shutdown(ssh2connection, "abnormal");
                exit(-1);
            }
            // A new remote process launched.
            char localhost[100];
            bzero(localhost, 100);
            gethostname(localhost, 100);
            int localhost_size = strlen(localhost);
            ssh2connection->write((char *)&localhost_size, int(sizeof(int)));
            ssh2connection->write(localhost, localhost_size);

            Socket socket(port); // server socket for this mprocess
            int sd = socket.getServerSocket();
            // The corresponding MNode created
	    cerr << "this sd = " << sd << endl;
            mNodes.push_back(new MNode(currHostName, pid, ssh2connection, sd));
        }

	if (printOutput == true)
	  cerr << "launchd mprocess at " << currHostName << endl;
    }

    // Handle nProc
    if (nProc < 0 || nProc > int(hosts.size()))
      nProc = hosts.size() + 1;  // count the master node
    systemSize = nProc;

    initializeThreads(nThr);
    INITIALIZED = true;

    // Synchronize with all slave processes
    for (int i = 0; i < int(hosts.size()); i++) {
      if (printOutput == true)
	cerr << "init: wait for ack from "
	     << mNodes[i]->getHostName() << endl;
      
      Message *m = mNodes[i]->receiveMessage();
      if (m->getAction() != Message::ACK) {
	cerr << "init didn't receive ack from rank " << (i + 1)
	     << " at " << mNodes[i]->getHostName() << endl;
	exit(-1);
      }
      delete m;
    }
    cerr << "MASS::init: done" << endl;
}

/**
 * Finishes computation.
 */
void MASS::finish() {
    Mthread::resumeThreads(Mthread::STATUS_TERMINATE);
    Mthread::barrierThreads(0);
    if (printOutput == true)
        cerr << "MASS::finish: all MASS threads terminated" << endl;

    // Close connection and finish each mprocess
    for (int i = 0; i < int(mNodes.size()); i++) {
        // Send a finish messages
        Message *m = new Message(Message::FINISH);
        mNodes[i]->sendMessage(m);
        delete m;
    }
    // Synchronize with all slaves
    barrier_all_slaves();
    // Close connection and finish each mprocess
    for (int i = 0; i < int(mNodes.size()); i++) {
        mNodes[i]->closeMainConnection();
        Ssh2Connection *connectionToClose = mNodes[i]->getConnection();
        util.shutdown(connectionToClose, "normal shutdown");
    }
    cerr << "MASS::finish: done" << endl;
}

/**
 *
 * @param return_values
 * @param stripe
 * @param arg_size
 * @param localAgents
 */
void MASS::barrier_all_slaves(char *return_values, int stripe, int arg_size,
                              int localAgents[]) {
    // counts the agent population from each Mprocess
    int nAgentsSoFar = (localAgents != NULL) ? localAgents[0] : 0;

    // Synchronize with all slave processes
    for (int i = 0; i < int(mNodes.size()); i++) {
        if (printOutput == true)
            cerr << "barrier waits for ack from " << mNodes[i]->getHostName()
                 << endl;
        Message *m = mNodes[i]->receiveMessage();

        if (printOutput == true)
            cerr << "barrier received a message from "
                 << mNodes[i]->getHostName() << "...message = " << m << endl;

        // check this is an Ack
        if (m->getAction() != Message::ACK) {
            cerr << "barrier didn't receive ack from rank " << (i + 1) << " at "
                 << mNodes[i]->getHostName()
                 << " message action type = " << m->getAction() << endl;
            exit(-1);
        }

        // retrieve arguments back from each Mprocess
        if (return_values != NULL && arg_size > 0) {
            if (stripe > 0 && localAgents == NULL) {
                // places.callAll( ) with return values
                m->getArgument(return_values + arg_size * stripe * (i + 1));
            }
            if (stripe == 0 && localAgents != NULL) {
                // agents.callAll( ) with return values
                m->getArgument(return_values + arg_size * nAgentsSoFar);
            }
        }

        // retrieve agent population from each Mprocess
        if (printOutput == true) {
            cerr << "localAgents[" << (i + 1)
                 << "] = m->getAgentPopulation: " << m->getAgentPopulation()
                 << endl;
        }

        if (localAgents != NULL) {
            localAgents[i + 1] = m->getAgentPopulation();
            nAgentsSoFar += localAgents[i + 1];
        }

        if (printOutput == true) cerr << "message deleted" << endl;
        delete m;
    }
}
