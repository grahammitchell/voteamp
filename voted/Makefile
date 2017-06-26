CFLAGS = -Wall -s
DFLAGS = -DDEBUG
LIBS   = -lmhash

all:
	g++ $(CFLAGS) -o voted main.cc init.cc $(LIBS)

debug:
	g++ $(CFLAGS) $(DFLAGS) -o voted main.cc init.cc $(LIBS)

