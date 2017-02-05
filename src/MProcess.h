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

#ifndef MPROCESS_H
#define MPROCESS_H

#include <vector>
#include <string>
#include <map>
#include "Message.h"

using namespace std;

class MProcess {
public:
	MProcess(char *hostName, int myPid, int nProc, int nThr, int port);
	void start();
private:
	string *hostName; // my local host name 
	int myPid;        // my pid or rank
	int nProc;        // # processes
	int nThr;         // # threads
	int port;         // TCP port
	int sd;           // bare TCP socket to the master
	vector<string> hosts;  // all hosts participated in computation

	void sendAck();
	void sendAck(int localPopulation);
	void sendReturnValues(void *argument, int nPlaces, int return_size);
	void sendReturnValues(void *argument, int nAgents, int return_size,
		int localPopulation);
	void sendMessage(Message *msg);
	Message *receiveMessage();
};

#endif
