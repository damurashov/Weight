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

#include <stdlib.h>    // getenv
#include "Utilities.h"
#include "Socket.h"
#include "Ssh2Connection.h"

const bool printOutput = false;
//const bool printOutput = true;

Utilities::Utilities() : passphrase("") {
	const char* homedir = getenv("HOME");
	if (homedir == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}
	keyfile1 = new char[150];
	keyfile2 = new char[150];
	strcpy(keyfile2, homedir);
	strcat(keyfile2, "/.ssh/id_rsa");
	strcpy(keyfile1, keyfile2);
	strcat(keyfile2, "\0");
	strcat(keyfile1, ".pub\0");
}


static void kbd_callback(const char *name, int name_len,
	const char *instruction, int instruction_len,
	int num_prompts,
	const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
	LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses,
	void **abstract) {
	string password_cpp = "password";
	const char *password = password_cpp.c_str();
	(void)name;
	(void)name_len;
	(void)instruction;
	(void)instruction_len;
	if (num_prompts == 1) {
		responses[0].text = strdup(password);
		responses[0].length = strlen(password);
	}
	(void)prompts;
	(void)abstract;
} /* kbd_callback */

int Utilities::waitsocket(int socket_fd, LIBSSH2_SESSION *session) {
	struct timeval timeout;
	int rc;
	fd_set fd;
	fd_set *writefd = NULL;
	fd_set *readfd = NULL;
	int dir;

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	FD_ZERO(&fd);

	FD_SET(socket_fd, &fd);

	/* now make sure we wait in the correct direction */
	dir = libssh2_session_block_directions(session);

	if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
		readfd = &fd;

	if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
		writefd = &fd;

	rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

	return rc;
}

Ssh2Connection *Utilities::establishConnection(const char host[],
	const int port,
	const char username[],
	const char password[]) {
	// Establish a TCP connection to host[]
	Socket *socket = new Socket(port);
	int sock = socket->getClientSocket(host);
	if (sock < 3) {
		cerr << "failed to connect!" << endl;
		exit(-1);
	}
	if (printOutput) {
		cerr << "socket to " << host << " created" << endl;
	}

	// Create a session instance
	LIBSSH2_SESSION *session = libssh2_session_init();
	libssh2_session_set_timeout(session, 2000);
	int return_code = 0;

	for (int i = 0; i < 5; i++) {
		if ((return_code = libssh2_session_handshake(session, sock)) == 0)
			break;
		if (printOutput)
			cerr << "session handshake: retry " << i << endl;
	}
	if (return_code) {
		shutdown(socket, session, NULL, "failure stablishing SSH session\0");
		exit(-1);
	}
	if (printOutput)
		cerr << "session created" << endl;

	// Check the fingerprint against our know hosts.
	// TODO: do we need this?
	const char *fingerprint = libssh2_hostkey_hash(session,
		LIBSSH2_HOSTKEY_HASH_SHA1);
	if (printOutput) {
		for (int i = 0; i < 20; i++) {
			fprintf(stderr, "%02x ", (unsigned char)fingerprint[i]);
		}
		cerr << endl;
	}

	// Check what authentication methods are available.
	char *userauthlist = libssh2_userauth_list(session, username, strlen(username));
	if (printOutput) {
		cerr << "Available ssh auth types: " << userauthlist << endl;
	}
	int auth_pw = 0;
	if (strstr(userauthlist, "password") != NULL)
		auth_pw |= 1;
	if (strstr(userauthlist, "keyboard-interactive") != NULL)
		auth_pw |= 2;
	if (strstr(userauthlist, "publickey") != NULL)
		auth_pw |= 4;

	// Authenticate a user
	if (auth_pw & 1) {     // Authenticate via password

		int retVal = 0;
		for (int i = 0; i < 5; i++) {
			int retVal = libssh2_userauth_password(session, username, password);
			if (retVal == 0) break;
			if (printOutput) {
				cerr << "retry: " << (i + 1) << endl;
			}
		}
		if (retVal != 0) {
			shutdown(socket, session, NULL, "password authentication failed\0");
			exit(-1);
		}
	}
	else if (auth_pw & 2) { // Or via keyboard-interactive
		if (libssh2_userauth_keyboard_interactive(session, username,
			&kbd_callback)) {
			shutdown(socket, session, NULL,
				"keyboard-interactive authentication failed");
			exit(-1);
		}
	}
	else if (auth_pw & 4) { // Or by public key
		if (libssh2_userauth_publickey_fromfile(session, username, keyfile1,
			keyfile2, passphrase)) {
			shutdown(socket, session, NULL, "public-key authentication failed");
			exit(-1);
		}
	}

	// Request a shell
	LIBSSH2_CHANNEL *channel = NULL;
	if (!(channel = libssh2_channel_open_session(session))) {
		shutdown(socket, session, channel, "Unable to open a session");
		exit(-1);
	}

	Ssh2Connection *ssh2connection = new Ssh2Connection(socket, session,
		channel);
	return ssh2connection;
}

bool Utilities::launchRemoteProcess(const Ssh2Connection *ssh2connection,
	const char cmd[]) {
	if (printOutput)
		cerr << "launch a remote process: " << cmd << endl;
	int return_code = 0;
	while ((return_code = libssh2_channel_exec(ssh2connection->channel, cmd))
		== LIBSSH2_ERROR_EAGAIN) {
		waitsocket(ssh2connection->socket->getDescriptor(),
			ssh2connection->session);
	}

	if (return_code != 0) {
		shutdown(ssh2connection, "error in launching remote process");
		return false;
	}

	return true;
}

void Utilities::shutdown(const Ssh2Connection *ssh2connection,
	const char msg[]) {
	shutdown(ssh2connection->socket, ssh2connection->session,
		ssh2connection->channel, msg);
	delete ssh2connection;
}

void Utilities::shutdown(Socket *socket,
	LIBSSH2_SESSION *session,
	LIBSSH2_CHANNEL *channel,
	const char msg[]) {
	if (msg != NULL)
		cerr << msg << endl;
	if (channel != NULL) {
		libssh2_channel_close(channel);
		libssh2_channel_free(channel);
		if (printOutput)
			cerr << "channel released" << endl;
	}
	if (session != NULL) {
		libssh2_session_disconnect(session, "Normal Shutdown");
		libssh2_session_free(session);
		if (printOutput)
			cerr << "session released" << endl;
	}
	if (socket != NULL) {
		delete socket;
		if (printOutput)
			cerr << "socket disconnected" << endl;
	}
}
