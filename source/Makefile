# Makefile for MAS
#g++ -std=c++11 -I ~/go/source/boost_1_73_0 example.cpp -o example
CC = g++ -std=c++11      #g++ #g++ -std=c++11 compiler
LIBMASS = libmass.so
MPROCESS = mprocess
GRAPH_OBJECTS = GraphPlaces.o

COMMON   = MASS_base.o Mthread.o Message.o Places_base.o Agents_base.o DllClass.o ExchangeHelper.o Socket.o Place.o Agent.o  GraphModel.o VertexModel.o VertexPlace.o FileParser.o File.o TxtFile.o NetcdfFile.o
MASS_OBJS = Utilities.o Ssh2Connection.o Places.o MASS.o MNode.o Agents.o $(COMMON) $(GRAPH_OBJECTS) 
prefix=./ssh2
exec_prefix=$(prefix)
libdir=$(exec_prefix)/lib
includedir=$(prefix)/include
SOURCE=../source


# define options, currently only LOGGING=1
ifeq ($(LOGGING), 1)
OPTIONS = -DLOGGING
endif

#TODO: auto detect rehl vs ubuntu

all: $(LIBMASS) $(MPROCESS)
	rm *.o

$(LIBMASS): $(MASS_OBJS)
	$(CC) $(OPTIONS) $(MASS_OBJS) -fPIC -L$(libdir) -lssh2 -lpthread -rdynamic -ldl -shared -o $@ #libmass.so

$(MPROCESS): MProcess.o
	$(CC) $(OPTIONS) MProcess.o $(COMMON) -fPIC -lpthread -rdynamic -ldl -o $@ # mprocess

clean: 
	rm -f *.o $(TARGET) $(LIBMASS) $(MPROCESS) $(WAVE2D) $(NOMAD) *.gch

%.o : $(SOURCE)/%.cpp   
	$(CC) $(OPTIONS) -Wall -I$(includedir)  -fPIC -c $< 

MProcess.o: $(SOURCE)/MProcess.h $(SOURCE)/MASS_base.h $(SOURCE)/Message.h $(SOURCE)/GraphPlaces.h 
MASS.o: $(SOURCE)/MASS_base.h $(SOURCE)/MASS.h $(SOURCE)/Utilities.h $(SOURCE)/Ssh2Connection.h $(SOURCE)/MNode.h $(SOURCE)/Places.h 
MASS_base.o: $(SOURCE)/MASS_base.h $(SOURCE)/Mthread.h $(SOURCE)/Places_base.h $(SOURCE)/AgentMigrationRequest.h
Utilities.o: $(SOURCE)/Utilities.h $(SOURCE)/Socket.h $(SOURCE)/Ssh2Connection.h
Socket.o: $(SOURCE)/Socket.h
MNode.o: $(SOURCE)/MNode.h $(SOURCE)/Ssh2Connection.h $(SOURCE)/Message.h 
Mthread.o: $(SOURCE)/Mthread.h $(SOURCE)/MASS_base.h
Message.o: $(SOURCE)/Message.h $(SOURCE)/RemoteExchangeRequest.h $(SOURCE)/AgentMigrationRequest.h 
Ssh2Connection.o: $(SOURCE)/Ssh2Connection.h
Place.o: $(SOURCE)/Place.h $(SOURCE)/MObject.h $(SOURCE)/File.h $(SOURCE)/TxtFile.h $(SOURCE)/NetcdfFile.h
Places.o: $(SOURCE)/Places.h $(SOURCE)/MObject.h $(SOURCE)/Place.h $(SOURCE)/Message.h
Places_base.o: $(SOURCE)/Places_base.h $(SOURCE)/MObject.h $(SOURCE)/Place.h $(SOURCE)/Message.h $(SOURCE)/FileParser.h 
Agent.o: $(SOURCE)/Agent.h $(SOURCE)/MObject.h $(SOURCE)/Place.h 
Agents.o: $(SOURCE)/Agents.h $(SOURCE)/MObject.h $(SOURCE)/Place.h $(SOURCE)/Message.h 
Agents_base.o: $(SOURCE)/Agents_base.h $(SOURCE)/MObject.h $(SOURCE)/Agent.h $(SOURCE)/AgentMigrationRequest.h
DllClass.o: $(SOURCE)/DllClass.h $(SOURCE)/MObject.h $(SOURCE)/Place.h 
ExchangeHelper.o: $(SOURCE)/Socket.h $(SOURCE)/Message.h  

#Graph feature headers
FileParser.o: $(SOURCE)/FileParser.h   
GraphModel.o: $(SOURCE)/GraphModel.h $(SOURCE)/VertexModel.h
GraphPlaces.o: $(SOURCE)/GraphPlaces.h $(SOURCE)/MASS.h  $(SOURCE)/VertexPlace.h $(SOURCE)/Graph.h $(SOURCE)/GraphModel.h $(SOURCE)/Message.h  $(SOURCE)/FileParser.h
VertexModel.o: $(SOURCE)/VertexModel.h $(SOURCE)/VertexModel.h
VertexPlace.o: $(SOURCE)/VertexPlace.h $(SOURCE)/FileParser.h $(SOURCE)/MASS_base.h $(SOURCE)/Place.h
File.o: $(SOURCE)/File.h $(SOURCE)/MASS_base.h $(SOURCE)/Places_base.h
TxtFile.o: $(SOURCE)/TxtFile.h $(SOURCE)/File.h
NetcdfFile.o: $(SOURCE)/NetcdfFile.h $(SOURCE)/File.h
