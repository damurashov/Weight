#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/net/metis/home3/dslab/MASS/c++/ssh2/lib
./mass dslab ds1ab-302 machinefile.txt 12345 5 4
