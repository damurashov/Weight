DIR_MASS:=../..
DIR_INCLUDE_SSH:=$(DIR_MASS)/lib/ssh2/include

GEN_MAIN:=main
GEN_LIFE:=Life
GEN=$(GEN_MAIN) $(GEN_LIFE)

all:
	g++ -Wall Life.cpp -I$(DIR_MASS)/source -shared -fPIC -std=c++11 -o $(GEN_LIFE) -L$(DIR_MASS)
	g++ -std=c++11 -Wall main.cpp Timer.cpp -I$(DIR_MASS)/source -L$(DIR_MASS) -lmass \
		-I$(DIR_INCLUDE_SSH) -lssh2 -o $(GEN_MAIN) -Wl,--verbose,-rpath,"\$$ORIGIN/$(DIR_MASS)"


clean:
	rm -f $(GEN)
