#!/usr/bin/env sh

export MASS_DIR=../../..

#OPTS=-DLOGGING


g++ $OPTS -Wall Wave2DMassPlace.cpp -I$MASS_DIR/source -shared -fPIC -o Wave2DMassPlace
g++ $OPTS -Wall main.cpp Timer.cpp -I$MASS_DIR/source -L$MASS_DIR/ubuntu -lmass -I$MASS_DIR/ubuntu/ssh2/include -L$MASS_DIR/ubuntu/ssh2/lib -lssh2 -o main
