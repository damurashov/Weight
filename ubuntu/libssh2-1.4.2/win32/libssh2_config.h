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

#ifndef LIBSSH2_CONFIG_H
#define LIBSSH2_CONFIG_H

#ifndef WIN32
#define WIN32
#endif
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif /* _CRT_SECURE_NO_DEPRECATE */
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#ifdef __MINGW32__
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#define HAVE_SYS_TIME_H
#endif

#define HAVE_WINSOCK2_H
#define HAVE_IOCTLSOCKET
#define HAVE_SELECT

#ifdef _MSC_VER
#define snprintf _snprintf
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#define strdup _strdup
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#define strncasecmp strnicmp
#define strcasecmp stricmp
#endif /* _MSC_VER */

/* Enable newer diffie-hellman-group-exchange-sha1 syntax */
#define LIBSSH2_DH_GEX_NEW 1

#endif /* LIBSSH2_CONFIG_H */

