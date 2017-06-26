CC=g++
EXT=
WARN=-Wall
DEBUG=
LIBS=-L/usr/lib/mysql -lmysqlclient

all: 
	$(CC) -o blacklister$(EXT) main.cc $(LIBS) $(WARN)

clean:
	rm *.o

install:
	cp ./blacklister /usr/local/bin/blacklister
