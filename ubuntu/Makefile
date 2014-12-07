# Makefile for MAS

CC	= g++
TARGET  = main
LIBMASS = libmass.so
MPROCESS = mprocess
WAVE2D = Wave2D
NOMAD  = Nomad
COMMON   = MASS_base.o Mthread.o Message.o Places_base.o Agents_base.o DllClass.o ExchangeHelper.o Socket.o Place.o Agent.o
MASS_OBJS = Utilities.o Ssh2Connection.o MASS.o MNode.o Places.o Agents.o $(COMMON)
prefix=/net/metis/home3/dslab/MASS/c++/ubuntu/ssh2
exec_prefix=$(prefix)
libdir=$(exec_prefix)/lib
includedir=$(prefix)/include
SOURCE=../source

all:  $(TARGET) $(LIBMASS) $(MPROCESS) $(WAVE2D) $(NOMAD)
	rm -f *.o

$(TARGET): main.o $(LIBMASS) 
	$(CC) main.o -L. -lmass -L$(libdir) -lssh2 -o $@ # main

$(LIBMASS): $(MASS_OBJS)
	$(CC) $(MASS_OBJS) -fPIC -L$(libdir) -lssh2 -lpthread -rdynamic -ldl -shared -o $@ #libmass.so

$(MPROCESS): MProcess.o $(COMMON)
	$(CC) MProcess.o $(COMMON) -fPIC -lpthread -rdynamic -ldl -o $@ # mprocess

$(WAVE2D): Wave2D.o
	$(CC) -shared Wave2D.o -fPIC -o $@ # Wave2D

$(NOMAD): Nomad.o
	$(CC) -shared Nomad.o -fPIC -o $@ # Nomad

clean: 
	rm -f *.o $(TARGET) $(LIBMASS) $(MPROCESS) $(WAVE2D) $(NOMAD)

%.o : $(SOURCE)/%.cpp
	$(CC) -Wall -I$(includedir) -fPIC -c $< 

main.o: $(SOURCE)/MASS.h
MProcess.o: $(SOURCE)/MProcess.h $(SOURCE)/MASS_base.h $(SOURCE)/Message.h
MASS.o: $(SOURCE)/MASS_base.h $(SOURCE)/MASS.h $(SOURCE)/Utilities.h $(SOURCE)/Ssh2Connection.h $(SOURCE)/MNode.h $(SOURCE)/Places.h 
MASS_base.o: $(SOURCE)/MASS_base.h $(SOURCE)/Mthread.h $(SOURCE)/Places_base.h $(SOURCE)/AgentMigrationRequest.h
Utilities.o: $(SOURCE)/Utilities.h $(SOURCE)/Socket.h $(SOURCE)/Ssh2Connection.h
Socket.o: $(SOURCE)/Socket.h
MNode.o: $(SOURCE)/MNode.h $(SOURCE)/Ssh2Connection.h $(SOURCE)/Message.h
Mthread.o: $(SOURCE)/Mthread.h $(SOURCE)/MASS_base.h
Message.o: $(SOURCE)/Message.h $(SOURCE)/RemoteExchangeRequest.h $(SOURCE)/AgentMigrationRequest.h
Ssh2Connection.o: $(SOURCE)/Ssh2Connection.h
Place.o: $(SOURCE)/Place.h $(SOURCE)/MObject.h
Places.o: $(SOURCE)/Places.h $(SOURCE)/MObject.h $(SOURCE)/Place.h $(SOURCE)/Message.h
Places_base.o: $(SOURCE)/Places_base.h $(SOURCE)/MObject.h $(SOURCE)/Place.h $(SOURCE)/Message.h
Agent.o: $(SOURCE)/Agent.h $(SOURCE)/MObject.h $(SOURCE)/Place.h
Agents.o: $(SOURCE)/Agents.h $(SOURCE)/MObject.h $(SOURCE)/Place.h $(SOURCE)/Message.h
Agents_base.o: $(SOURCE)/Agents_base.h $(SOURCE)/MObject.h $(SOURCE)/Agent.h $(SOURCE)/Nomad.h $(SOURCE)/AgentMigrationRequest.h
DllClass.o: $(SOURCE)/DllClass.h $(SOURCE)/MObject.h $(SOURCE)/Place.h
ExchangeHelper.o: $(SOURCE)/Socket.h $(SOURCE)/Message.h
Wave2D.o: $(SOURCE)/MASS_base.h $(SOURCE)/Place.h $(SOURCE)/Wave2D.h
Nomad.o: $(SOURCE)/MASS_base.h $(SOURCE)/Agent.h $(SOURCE)/Nomad.h




