#!/usr/bin/env sh

export MASS_DIR=../..

#OPTS=-DLOGGING

g++ $OPTS -Wall Wave2DMassPlace.cpp -I$MASS_DIR/src -shared -fPIC -o Wave2DMassPlace
g++ $OPTS -Wall Wave2D.cpp Timer.cpp -I$MASS_DIR/src -L$MASS_DIR/lib -lmass -I$MASS_DIR/lib/dependencies/ssh2/include -L$MASS_DIR/lib/dependencies/ssh2/lib -lssh2 -o Wave2D
