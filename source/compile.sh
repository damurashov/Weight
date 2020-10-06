#!/bin/sh
#export MASS_DIR=../mass_cpp_core
#g++ -Wall FilePlace.cpp -I$MASS_DIR/source/parallel_io -shared -fPIC -std=c++11 -o FilePlace
#g++ -Wall TxtFile.cpp -I$MASS_DIR/source/parallel_io -shared -fPIC -std=c++11 -o TxtFile
#g++ -std=c++11 -Wall main.cpp  -I$MASS_DIR/source -L$MASS_DIR/ubuntu -lmassTxtFileDIR/ubuntu/ssh2/include -L$MASS_DIR/ubuntu/ssh2/lib -lssh2 -o p_io

#Set up the following two shell variables
export MASS_DIR=/home/NETID/alabssie/dev/mass_cpp/mass_cpp_core

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/NETID/alabssie/dev/mass_cpp/mass_cpp_core/ubuntu/ssh2/lib:/home/NETID/alabssie/dev/mass_cpp/mass_cpp_core/ubuntu


#Compile your main program as well as all your Agents/Places-derived classes.
#To compile your program that includes main( ), say main.cpp,

g++ -Wall -std=c++11  main.cpp -I$MASS_DIR/source -L$MASS_DIR/ubuntu -lmass -I$MASS_DIR/ubuntu/ssh2/include -L$MASS_DIR/ubuntu/ssh2/lib -lssh2 -o main


#To compile your Agents/Places-derived class, say Land.cpp
#Note that you must compile all your Agents/Places-derived classes whose executable is dynamic-linked to mprocess 
#whenever your main program invokes new Places( ) or new Agents( )

#g++ -Wall FilePlace.cpp -I$MASS_DIR/source -shared -fPIC -o fileplace
g++ -Wall -std=c++11 TestPlace.cpp -I$MASS_DIR/source -shared -fPIC -o TestPlace
g++ -Wall -std=c++11 Test.cpp -I$MASS_DIR/source -shared -fPIC -o Test

