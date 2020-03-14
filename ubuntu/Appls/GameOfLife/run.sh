#!/usr/bin/env bash

#Below is load path for the library
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../..:../../ssh2/lib

echo "Number of nodes? {1, 2, 4, 8, 16}: "
read NUMNODES
echo "Number of threads? {1, 2, 4}: "
read NUMTHREADS
#echo "Number of turns? "
#read NUMTURNS
#echo "X size? "
#read XSIZE
#echo "Y size? "
#read YSIZE
#echo "Port? "
#read PORT
#echo "Password? "
#read -s PASSWORD

head -$(($NUMNODES-1)) machinefile.txt > .tempmachinefile.txt

./main $USER UPD8ur5wk!! .tempmachinefile.txt 12345 $NUMNODES $NUMTHREADS 100 10 10

rm .tempmachinefile.txt
