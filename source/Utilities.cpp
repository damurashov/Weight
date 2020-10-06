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

#include "Utilities.h"
#include <stdlib.h>  // getenv
#include "Socket.h"
#include "Ssh2Connection.h"
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = false;
#endif
/**
 *
 * @param name
 * @param name_len
 * @param instruction
 * @param instruction_len
 * @param num_prompts
 * @param prompts
 * @param responses
 * @param abstract
 */
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
}

Utilities::Utilities() : passphrase("") {
    const char *homedir = getenv("HOME");
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

/**
 *
 * @param socket_fd
 * @param session
 * @return
 */
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

    if (dir & LIBSSH2_SESSION_BLOCK_INBOUND) readfd = &fd;

    if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

/**
 *
 * @param host
 * @param port
 * @param username
 * @param password
 * @return
 */
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
    cerr << "socket to " << host << " created" << endl;

    // Create a session instance
    LIBSSH2_SESSION *session = libssh2_session_init();
    if ( !session ) {
      cerr << "session not created" << endl;
      exit( -1 );
    }
      
    libssh2_session_set_timeout(session, 2000);
    int return_code = 0;

    /* This preference no longer works
    if ((return_code = libssh2_session_method_pref(
             session, LIBSSH2_METHOD_CRYPT_CS, "arcfour")) != 0) {
        cerr << "session method preference CS error" << endl;
        exit(-1);
    }
    */

    for (int i = 0; i < 10; i++) {
        if ((return_code = libssh2_session_handshake(session, sock)) == 0)
            break;
	printErrorMessage( return_code );
        cerr << "session handshake: retry " << i << endl;
    }
    if (return_code) {

        shutdown(socket, session, NULL, "failure establishing SSH session\0");
        exit(-1);
    }
    cerr << "session created" << endl;

    // Check the fingerprint against our know hosts.
    // TODO: do we need this?
    const char *fingerprint =
        libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    for (int i = 0; i < 20; i++) {
        printf("%02x ", (unsigned char)fingerprint[i]);
    }
    cout << endl;

    // Check what authentication methods are available.
    char *userauthlist =
        libssh2_userauth_list(session, username, strlen(username));
    cout << userauthlist << endl;
    int auth_pw = 0;
    if (strstr(userauthlist, "password") != NULL) auth_pw |= 1;
    if (strstr(userauthlist, "keyboard-interactive") != NULL) auth_pw |= 2;
    if (strstr(userauthlist, "publickey") != NULL) auth_pw |= 4;

    // Authenticate a user
    if (auth_pw & 1) {  // Authenticate via password

        int retVal = 0;
        for (int i = 0; i < 10; i++) {
            int retVal = libssh2_userauth_password(session, username, password);
            if (retVal == 0) break;

	    printErrorMessage( retVal );
            cerr << "userauth password: retry " << (i + 1) << endl;
            continue;
        }
        if (retVal != 0) {
            shutdown(socket, session, NULL, "password authentication failed\0");
            exit(-1);
        }
    } else if (auth_pw & 2) {  // Or via keyboard-interactive
        if (libssh2_userauth_keyboard_interactive(session, username,
                                                  &kbd_callback)) {
            shutdown(socket, session, NULL,
                     "keyboard-interactive authentication failed");
            exit(-1);
        }
    } else if (auth_pw & 4) {  // Or by public key
        if (libssh2_userauth_publickey_fromfile(session, username, keyfile1,
                                                keyfile2, password)) {
            shutdown(socket, session, NULL, "public-key authentication failed");
            exit(-1);
        }
    }

    // Request a shell
    LIBSSH2_CHANNEL *channel = NULL;
    for ( int i = 0; i < 10; i++ ) {
      if (!(channel = libssh2_channel_open_session(session))) {
	printErrorMessage( libssh2_session_last_errno( session ) );
        cerr << "channel open: retry " << i << endl;	
      }
    }
    if ( !channel ) {
        shutdown(socket, session, channel, "Unable to open a session");
        exit(-1);
    }

    Ssh2Connection *ssh2connection =
        new Ssh2Connection(socket, session, channel);
    return ssh2connection;
}

/**
 * @param return_code
 */
void Utilities::printErrorMessage( int return_code ) {
  switch( return_code ) {
  case LIBSSH2_ERROR_SOCKET_NONE: cerr << "LIBSSH2_ERROR_SOCKET_NONE" << endl; break;
  case LIBSSH2_ERROR_BANNER_RECV: cerr << "LIBSSH2_ERROR_BANNER_RECV" << endl; break;
  case LIBSSH2_ERROR_BANNER_SEND: cerr << "LIBSSH2_ERROR_BANNER_SEND" << endl; break;
  case LIBSSH2_ERROR_INVALID_MAC: cerr << "LIBSSH2_ERROR_INVALID_MAC" << endl; break;
  case LIBSSH2_ERROR_KEX_FAILURE: cerr << "LIBSSH2_ERROR_KEX_FAILURE" << endl; break;
  case LIBSSH2_ERROR_ALLOC: cerr << "LIBSSH2_ERROR_ALLOC" << endl; break;
  case LIBSSH2_ERROR_SOCKET_SEND: cerr << "LIBSSH2_ERROR_SOCKET_SEND" << endl; break;
  case LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE: cerr << "LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE" << endl; break;
  case LIBSSH2_ERROR_TIMEOUT: cerr << "LIBSSH2_ERROR_TIMEOUT" << endl; break;
  case LIBSSH2_ERROR_HOSTKEY_INIT: cerr << "LIBSSH2_ERROR_HOSTKEY_INIT" << endl; break;
  case LIBSSH2_ERROR_HOSTKEY_SIGN: cerr << "LIBSSH2_ERROR_HOSTKEY_SIGN" << endl; break;
  case LIBSSH2_ERROR_DECRYPT: cerr << "LIBSSH2_ERROR_DECRYPT" << endl; break;
  case LIBSSH2_ERROR_SOCKET_DISCONNECT: cerr << "LIBSSH2_ERROR_SOCKET_DISCONNECT" << endl; break;
  case LIBSSH2_ERROR_PROTO: cerr << "LIBSSH2_ERROR_PROTO" << endl; break;
  case LIBSSH2_ERROR_PASSWORD_EXPIRED: cerr << "LIBSSH2_ERROR_PASSWORD_EXPIRED" << endl; break;
  case LIBSSH2_ERROR_FILE: cerr << "LIBSSH2_ERROR_FILE" << endl; break;
  case LIBSSH2_ERROR_METHOD_NONE: cerr << "LIBSSH2_ERROR_METHOD_NONE" << endl; break;
  case LIBSSH2_ERROR_AUTHENTICATION_FAILED: 
    cerr << "LIBSSH2_ERROR_AUTHENTICATION_FAILED" << endl;
    cerr << "LIBSSH2_ERROR_PUBLICKEY_UNRECOGNIZED" << endl;
    cerr << "LIBSSH2_ERROR_AUTHENTICATION_FAILED" << endl; break;
  case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED: cerr << "LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_OUTOFORDER: cerr << "LIBSSH2_ERROR_CHANNEL_OUTOFORDER" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_FAILURE: cerr << "LIBSSH2_ERROR_CHANNEL_FAILURE" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: cerr << "LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_UNKNOWN: cerr << "LIBSSH2_ERROR_CHANNEL_UNKNOWN" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED: cerr << "LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED: cerr << "LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_CLOSED: cerr << "LIBSSH2_ERROR_CHANNEL_CLOSED" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_EOF_SENT: cerr << "LIBSSH2_ERROR_CHANNEL_EOF_SENT" << endl; break;
  case LIBSSH2_ERROR_SCP_PROTOCOL: cerr << "LIBSSH2_ERROR_SCP_PROTOCOL" << endl; break;
  case LIBSSH2_ERROR_ZLIB: cerr << "LIBSSH2_ERROR_ZLIB" << endl; break;
  case LIBSSH2_ERROR_SOCKET_TIMEOUT: cerr << "LIBSSH2_ERROR_SOCKET_TIMEOUT" << endl; break;
  case LIBSSH2_ERROR_SFTP_PROTOCOL: cerr << "LIBSSH2_ERROR_SFTP_PROTOCOL" << endl; break;
  case LIBSSH2_ERROR_REQUEST_DENIED: cerr << "LIBSSH2_ERROR_REQUEST_DENIED" << endl; break;
  case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED: cerr << "LIBSSH2_ERROR_METHOD_NOT_SUPPORTED" << endl; break;
  case LIBSSH2_ERROR_INVAL: cerr << "LIBSSH2_ERROR_INVAL" << endl; break;
  case LIBSSH2_ERROR_INVALID_POLL_TYPE: cerr << "LIBSSH2_ERROR_INVALID_POLL_TYPE" << endl; break;
  case LIBSSH2_ERROR_PUBLICKEY_PROTOCOL: cerr << "LIBSSH2_ERROR_PUBLICKEY_PROTOCOL" << endl; break;
  case LIBSSH2_ERROR_EAGAIN: cerr << "LIBSSH2_ERROR_EAGAIN" << endl; break;
  case LIBSSH2_ERROR_BUFFER_TOO_SMALL: cerr << "LIBSSH2_ERROR_BUFFER_TOO_SMALL" << endl; break;
  case LIBSSH2_ERROR_BAD_USE: cerr << "LIBSSH2_ERROR_BAD_USE" << endl; break;
  case LIBSSH2_ERROR_COMPRESS: cerr << "LIBSSH2_ERROR_COMPRESS" << endl; break;
  case LIBSSH2_ERROR_OUT_OF_BOUNDARY: cerr << "LIBSSH2_ERROR_OUT_OF_BOUNDARY" << endl; break;
  case LIBSSH2_ERROR_AGENT_PROTOCOL: cerr << "LIBSSH2_ERROR_AGENT_PROTOCOL" << endl; break;
  case LIBSSH2_ERROR_SOCKET_RECV: cerr << "LIBSSH2_ERROR_SOCKET_RECV" << endl; break;
  case LIBSSH2_ERROR_ENCRYPT: cerr << "LIBSSH2_ERROR_ENCRYPT" << endl; break;
  case LIBSSH2_ERROR_BAD_SOCKET: cerr << "LIBSSH2_ERROR_BAD_SOCKET" << endl; break;
  case LIBSSH2_ERROR_KNOWN_HOSTS: cerr << "LIBSSH2_ERROR_KNOWN_HOSTS" << endl; break;
  case LIBSSH2_ERROR_CHANNEL_WINDOW_FULL: cerr << "LIBSSH2_ERROR_CHANNEL_WINDOW_FULL" << endl; break;
  case LIBSSH2_ERROR_KEYFILE_AUTH_FAILED: cerr << "LIBSSH2_ERROR_KEYFILE_AUTH_FAILED" << endl; break;
  }
}

/**
 *
 * @param ssh2connection
 * @param cmd
 * @return
 */
bool Utilities::launchRemoteProcess(const Ssh2Connection *ssh2connection,
                                    const char cmd[]) {
    cerr << "launch a remote process: " << cmd << endl;
    int return_code = 0;
    while ((return_code = libssh2_channel_exec(ssh2connection->channel, cmd)) ==
	   LIBSSH2_ERROR_EAGAIN || return_code == LIBSSH2_ERROR_TIMEOUT ) {
      waitsocket(ssh2connection->socket->getDescriptor(),
		 ssh2connection->session);
      printErrorMessage( return_code );
    }
    if (return_code != 0) {
	printErrorMessage( return_code );
        shutdown(ssh2connection, "error in remote execution");
        return false;
    }
    return true;
}

/**
 *
 * @param ssh2connection
 * @param msg
 */
void Utilities::shutdown(const Ssh2Connection *ssh2connection,
                         const char msg[]) {
    shutdown(ssh2connection->socket, ssh2connection->session,
             ssh2connection->channel, msg);
    delete ssh2connection;
}

/**
 *
 * @param socket
 * @param session
 * @param channel
 * @param msg
 */
void Utilities::shutdown(Socket *socket, LIBSSH2_SESSION *session,
                         LIBSSH2_CHANNEL *channel, const char msg[]) {
    if (msg != NULL) cerr << msg << endl;
    if (channel != NULL) {
        libssh2_channel_close(channel);
        libssh2_channel_free(channel);
        cerr << "channel released" << endl;
    }
    if (session != NULL) {
        libssh2_session_disconnect(session, "Normal Shutdown");
        libssh2_session_free(session);
        cerr << "session released" << endl;
    }
    if (socket != NULL) {
        delete socket;
        cerr << "socket disconnected" << endl;
    }
}
