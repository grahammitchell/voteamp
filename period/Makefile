CC=g++
EXT=
WARN=-Wall
DEBUG=

all: main.o init.o globals.o
	$(CC) -o period$(EXT) main.o init.o globals.o $(WARN)

main.o: main.cc init.h globals.h vector.cpp
	$(CC) -o main.o -c main.cc $(WARN) $(DEBUG)

init.o: init.cc init.h globals.h vector.cpp
	$(CC) -o init.o -c init.cc $(WARN)

globals.o: globals.cc globals.h
	$(CC) -o globals.o -c globals.cc $(WARN)

clean:
	rm *.o
