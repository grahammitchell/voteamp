OBJ= main.o ui.o net.o
SRC= main.cc ui.cc net.cc globals.h net.h ui.h Makefile

all: voter

voter: $(OBJ)
	g++ -o voter -Wall $(OBJ) -lncurses

main.o: main.cc globals.h net.h ui.h
	g++ -o main.o -Wall -c main.cc

ui.o: ui.cc globals.h ui.h
	g++ -o ui.o -Wall -c ui.cc

net.o: net.cc globals.h net.h
	g++ -o net.o -Wall -c net.cc

clean:
	rm -f *.o
