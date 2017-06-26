all: main.cc
	g++ -o listmaker -Wall main.cc -L/usr/lib/mysql -lmysqlclient

debug:
	g++ -o listmaker -Wall -DDEBUG main.cc -L/usr/lib/mysql -lmysqlclient

install:
	cp ./listmaker /usr/local/bin/listmaker
