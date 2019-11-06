#!/usr/bin/env bash

# Below is the relative load path for the library
# if the location of this application directory is changed, this must be updated
<<<<<<< HEAD:ubuntu/Appls/Wave2D/run.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../ssh2/lib
=======
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../lib:../../lib/dependencies/ssh2/lib
>>>>>>> origin/cleanup:samples/Wave2D/run.sh

echo "Number of nodes? {1, 2, 4, 8, 16}: "
read NUMNODES
echo "Number of threads? {1, 2, 4}: "
read NUMTHREADS
echo "Number of turns? "
read NUMTURNS
echo "X size? "
read XSIZE
echo "Y size? "
read YSIZE
echo "Port? "
read PORT
echo "Password? "
read PASSWORD

head -$(($NUMNODES-1)) machinefile.txt > .tempmachinefile.txt

./Wave2D $USER $PASSWORD .tempmachinefile.txt $PORT $NUMNODES $NUMTHREADS $XSIZE $YSIZE $NUMTURNS

rm .tempmachinefile.txt
