#!/bin/sh

export MASS_DIR=../..

g++ -Wall Nomad.cpp -I$MASS_DIR/src -shared -fPIC -o Nomad
g++ -Wall Land.cpp -I$MASS_DIR/src -shared -fPIC -o Land
g++ -Wall main.cpp -I$MASS_DIR/src -L$MASS_DIR/lib -lmass -I$MASS_DIR/lib/dependencies/ssh2/include -L$MASS_DIR/lib/dependencies/ssh2/lib -lssh2 -o main
