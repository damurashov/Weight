#!/bin/sh

export MASS_DIR=/net/metis/home3/dslab/MASS/c++

g++ -Wall Nomad.cpp -I$MASS_DIR/source -shared -fPIC -o Nomad
g++ -Wall Land.cpp -I$MASS_DIR/source -shared -fPIC -o Land
g++ -Wall main.cpp -I$MASS_DIR/source -L$MASS_DIR/ubuntu -lmass -I$MASS_DIR/ubuntu/ssh2/include -L$MASS_DIR/ubuntu/ssh2/lib -lssh2 -o main
