# MASS C++ Software License
# © 2014-2015 University of Washington
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
# © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


#!/bin/sh

# Written by Simon Josefsson.

# Start sshd, invoke parameters, saving exit code, kill sshd, and
# return exit code.

srcdir=${srcdir:-$PWD}
SSHD=${SSHD:-/usr/sbin/sshd}

cmd="./ssh2${EXEEXT}"
srcdir=`cd "$srcdir"; pwd`

PRIVKEY=$srcdir/etc/user
export PRIVKEY
PUBKEY=$srcdir/etc/user.pub
export PUBKEY

if test -n "$DEBUG"; then
    libssh2_sshd_params="-d -d"
fi

chmod go-rwx "$srcdir"/etc/host*
$SSHD -f /dev/null -h "$srcdir"/etc/host \
    -o 'Port 4711' \
    -o 'Protocol 2' \
    -o "AuthorizedKeysFile $srcdir/etc/user.pub" \
    -o 'UsePrivilegeSeparation no' \
    -o 'StrictModes no' \
    -D \
    $libssh2_sshd_params &
sshdpid=$!

trap "kill ${sshdpid}; echo signal killing sshd; exit 1;" EXIT

: "started sshd (${sshdpid})"

sleep 3

: Invoking $cmd...
eval $cmd
ec=$?
: Self-test exit code $ec

: "killing sshd (${sshdpid})"
kill "${sshdpid}" > /dev/null 2>&1
trap "" EXIT
exit $ec
