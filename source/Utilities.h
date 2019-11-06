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

#ifndef UTILITIES_H
#define UTILITIES_H

#include "libssh2_config.h"
#include <libssh2.h>
#include <unistd.h>
#include <iostream>         // cerr
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "Socket.h"         // Socket
#include "Ssh2Connection.h"

using namespace std;

/**
 * Utilities for the MASS library.
 * Includes use of sockets and SSH2
 * Mostly focuses on providing SSH2 connections between MProcesses.
 */
class Utilities {
public:
	Utilities();
	Ssh2Connection *establishConnection(const char host[],
		const int port,
		const char username[],
		const char password[]);
	bool launchRemoteProcess(const Ssh2Connection *ssh2connection,
		const char cmd[]);
	void shutdown(const Ssh2Connection *ssh2connection, const char msg[]);

private:
	char *keyfile1;
	char *keyfile2;
	const char *passphrase;

	int waitsocket(int socket_fd, LIBSSH2_SESSION *session);
	void shutdown(Socket *socket, LIBSSH2_SESSION *session,
		LIBSSH2_CHANNEL *channel, const char msg[]);
};

#endif
