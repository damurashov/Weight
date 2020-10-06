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

#include "MNode.h"
#include <sstream>  // ostringstream
#include "MASS_base.h"

// Used to toggle output for MNode
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

using namespace std;

/**
 * Not yet implemented.
 */
void MNode::closeMainConnection() { close(sd); }

/**
 * Method to send messages to individual MNode instances within a simulation.
 *
 * Each host used in a simulation is represented by an MNode, which provides an
 * interface to the various MProcess and MThread instances running on the host,
 * using a common messaging paradigm.
 *
 * @param m the address of (pointer) the actual Message Object to send
 */
void MNode::sendMessage(Message *m) {
    int size;
    char *buf = m->serialize(size);     
    if (printOutput) cerr << "MNode::sendMessage: size = " << size << endl;
    if (write(sd, (char *)&size, sizeof(int)) > 0) {
        if (write(sd, buf, size) > 0) {
            return;
        }
    }
    MASS_base::log("sendMessage error");
    exit(-1);
}

/**
 * Method to receive messages from individual MNode instances within a
 * simulation.
 *
 * Each host used in a simulation is represented by an MNode, which provides an
 * interface to the various MProcess and MThread instances running on the host,
 * using a common messaging paradigm.
 *
 * @return  the Message from this MNode
 */
Message *MNode::receiveMessage() {
    int size = -1;
    int nRead = 0;
    if ((nRead = read(sd, (char *)&size, sizeof(int))) > 0) {
        if (printOutput)
            cerr << "nRead = " << nRead << ", size = " << size << endl;
        char *buf = new char[size];
        for (nRead = 0; (nRead += read(sd, buf + nRead, size - nRead)) < size;)
            ;
        if (printOutput) cerr << "nRead = " << nRead << endl;
        if (size > 4) {
            if (printOutput)
                cerr << "*(int *)(buf + 4) = " << *(int *)(buf + 4) << endl;
        }
        Message *m = new Message();
        m->deserialize(buf, size);
        return m;
    } else if (nRead <= 0) {
        ostringstream convert;
        convert << "receivMessage error from rank[" << pid << "] at "
                << hostName;
        MASS_base::log(convert.str());
        exit(-1);
    }
    return NULL;
}

Ssh2Connection *MNode::getConnection() { return connection; }
