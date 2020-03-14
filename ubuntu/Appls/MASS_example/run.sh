#!/bin/sh
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/net/metis/home3/dslab/MASS/c++/ubuntu/ssh2/lib
#./main dslab ds1ab-302 machinefile.txt 12345 5 4
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../..:../../ssh2/lib

 echo "Number of nodes? {1, 2, 4, 8, 16}: "
 read NUMNODES
# echo "Number of threads? {1, 2, 4}: "
# read NUMTHREADS
# echo "Number of turns? "
# read NUMTURNS
# echo "X size? "
# read XSIZE
# echo "Y size? "
# read YSIZE
# echo "Port? "
# read PORT
# echo "Password? "
# read -s PASSWORD

# head -$(($NUMNODES-1)) machinefile.txt > .tempmachinefile.txt

# ./main $USER $PASSWORD .tempmachinefile.txt $PORT $NUMNODES $NUMTHREADS $XSIZE $YSIZE $NUMTURNS

head -$(($NUMNODES-1)) machinefile.txt > .tempmachinefile.txt

./main $USER UPD8ur5wk!! .tempmachinefile.txt 4445 $NUMNODES 1

rm .tempmachinefile.txt