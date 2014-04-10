# Makefile for MAS

CC	= g++
TARGET  = mass
MPROCESS = mprocess
WAVE2D = Wave2D
NOMAD  = Nomad
COMMON   = MASS_base.o Mthread.o Message.o Places_base.o Agents_base.o DllClass.o ExchangeHelper.o Socket.o Place.o Agent.o
MASS_OBJS = Utilities.o Ssh2Connection.o MASS.o MNode.o Places.o Agents.o $(COMMON)
prefix=/net/metis/home3/dslab/MASS/c++/ssh2
exec_prefix=$(prefix)
libdir=$(exec_prefix)/lib
includedir=$(prefix)/include

all:  $(TARGET) $(MPROCESS) $(WAVE2D) $(NOMAD)

$(TARGET): main.o $(MASS_OBJS) 
	$(CC) main.o $(MASS_OBJS) -L$(libdir) -lssh2 -lpthread -rdynamic -ldl -o $@ # mass

$(MPROCESS): MProcess.o $(COMMON)
	$(CC) MProcess.o $(COMMON) -lpthread -rdynamic -ldl -o $@ # mprocess

$(WAVE2D): Wave2D.o
	$(CC) -shared Wave2D.o -o $@ # Wave2D

$(NOMAD): Nomad.o
	$(CC) -shared Nomad.o -o $@ # Nomad

clean: 
	rm *.o $(WAVE2D)

.cpp.o:
	$(CC) -Wall -I$(includedir) -c $< 

main.o: MASS.h
MProcess.o: MProcess.h MASS_base.h Message.h
MASS.o: MASS_base.h MASS.h Utilities.h Ssh2Connection.h MNode.h Places.h 
MASS_base.o: MASS_base.h Mthread.h Places_base.h
Utilities.o: Utilities.h Socket.h Ssh2Connection.h
Socket.o: Socket.h
MNode.o: MNode.h Ssh2Connection.h Message.h
Mthread.o: Mthread.h MASS_base.h
Message.o: Message.h RemoteExchangeRequest.h
Ssh2Connection.o: Ssh2Connection.h
Place.o: Place.h MObject.h
Places.o: Places.h MObject.h Place.h Message.h
Places_base.o: Places_base.h MObject.h Place.h Message.h
Agent.o: Agent.h MObject.h Place.h
Agents.o: Agents.h MObject.h Place.h Message.h
Agents_base.o: Agents_base.h MObject.h Agent.h
DllClass.o: DllClass.h MObject.h Place.h
ExchangeHelper.o: Socket.h Message.h
Wave2D.o: MASS_base.h Place.h Wave2D.h
Nomad.o: MASS_base.h Agent.h Nomad.h




