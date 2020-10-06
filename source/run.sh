
export MASS_DIR=/home/NETID/alabssie/dev/mass_cpp/mass_cpp_core
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/NETID/alabssie/dev/mass_cpp/mass_cpp_core/ubuntu/ssh2/lib:/home/NETID/alabssie/dev/mass_cpp/mass_cpp_core/ubuntu


#echo "user name? "
#read username
#echo "password? "
#read password
#echo "machinefile"
#read machinefile
#read SIZE

#NUMNODES=4
#NUMTHREADS=1
#PASSWORD=$YOUR_PASSWORD

./main  args

#Compile your main program as well as all your Agents/Places-derived classes.
#To compile your program that includes main( ), say main.cpp,

#g++ -Wall main.cpp -I$MASS_DIR/source -L$MASS_DIR/ubuntu -lmass -I$MASS_DIR/ubuntu/ssh2/include -L$MASS_DIR/ubuntu/ssh2/lib -lssh2 -o main

#To compile your Agents/Places-derived class, say Land.cpp
#Note that you must compile all your Agents/Places-derived classes whose executable is dynamic-linked to mprocess
#whenever your main program invokes new Places( ) or new Agents( )

#g++ -Wall FilePlace.cpp -I$MASS_DIR/source -shared -fPIC -o fileplace
