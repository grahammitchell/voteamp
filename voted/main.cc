
// voteserver 0.1.1 (c) 1999 Graham Mitchell - 24 Jul 99

// main.cc
// The main program - a nonforking server/daemon

/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License (COPYING) for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software Foundation,
	Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>	// ceil()
#include <ctype.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <mhash.h>	// MD5 functions
#include "globals.h"
#include "init.h"

#if 0
#define FORKING
#else
#define NOFORKING
#endif

struct frame
{
	int value;
	int pos;
};

// globals from globals.h
int default_port = -1;
char* allowfilename;
char* denyfilename;
int auth_mode;
int voting_frequency;
int voting_threshold;
int voting_maximum;
int voting_duration;
int clamp_value;
char* ballotfilename;
char* statefilename;
char* resultsfilename;
bool verbose;
bool detach;

const char* welcome = "VoteAmp 0.1.2\n";
const u_short portbase = 0;
char* ballot_actions[200];	// FIXME: remove hard coded limit
char* ballot_names[200];	// FIXME: remove hard coded limit
bool already_voted[200];	// FIXME: you know what to do
int num_ballot_items = 0;
int num_users = 0;

struct user
{
	char* name;
	char passwd[33];
};

user users[200];			// FIXME: remove hard coded limit

void httpser( int );
int getvalue( char );
void init();
void shutdown();
void daemonize();
bool authorized( const char* , const char* );
void tally( char* );
void clamp( char* , int );
void write_results();
int framecmp(const void*, const void* );
void pairsort(char **, char **, int );
void rebuild(int);	// signal handler to restart voting
void die(int);		// signal handler to shutdown
void hash_passwd( char* raw, char* hashed );





/***********************************************************************
	Write "n" bytes to a stream socket.
************************************************************************/
int writen(int fd, char *ptr, int nbytes)
{
	int nleft, nwritten;

	nleft = nbytes;

	while (nleft > 0)
	{
		nwritten = write(fd, ptr, nleft);
		if ( nwritten <= 0 ) 
			return(nwritten);	// error

		nleft -= nwritten;
		ptr += nwritten;
	}

	return(nbytes - nleft);
}


/***********************************************************************
	Read a line from a descriptor. Store the new line in the buffer.
	Return the number of characters up to, but not including, the null.
************************************************************************/
int readline(int fd, char* ptr, int maxlen)
{
	int  n, rc;
	char c;

	for ( n=1 ; n<maxlen ; n++ )
	{
		if ( (rc = read(fd, &c, 1)) == 1 )
		{
			*ptr++ = c;
			if ( c == '\n' ) break;
		}
		else if ( rc == 0 )
		{
			if ( n == 1 )  return(0);   // EOF, no data read
			else break;					// EOF, some data was read
		}
		else return(-1);				// error
	}

	*ptr = 0;
	return n;
}


/************************************************************************
	Sends a named file line-by-line to the socket named by sockfd.
	The file is opened for reading and closed before returning.
*************************************************************************/
void send_file(int sockfd, char *filename)
{
	FILE *fd;
	char line[MAXLINE];
	int  n;

	fd = fopen(filename, "r");

	while (fgets(line, MAXLINE, fd) != NULL)
	{
		n = strlen(line);
		if(writen(sockfd, line, n) != n)
		{
			cerr << "voted send_file(): writen or socket error\n";  
			exit(3);
		}
	}

	if (ferror(fd))
		cerr << "voted send_file(): error reading file\n";

	fclose(fd);
}


/************************************************************************
	Sets up a listen server on the named port (default 4417).
	Creates a child process for each incoming request and creates
	a new socket for it.  Should be concurrent.
*************************************************************************/
int main(int argc, char **argv)
{
	int sockfd, newsockfd, childpid;
	unsigned clilen;
	struct sockaddr_in cli_addr, serv_addr;
	time_t current_time = 0, next_update = 0;
	ifstream statefile;
	int num_votes;

	// get values from ini file and/or command-line parameters
	config(argc, argv);

	// go ahead and detach ourselves to the background
	if ( detach )
		daemonize();

	// create pid file in /var/run/

	ofstream pidfile;
	pidfile.open("/var/run/voted.pid");		//FIXME: make this configurable
	pidfile << getpid() << endl;
	pidfile.close();

	signal(SIGHUP, rebuild);
	signal(SIGTERM, die);
	signal(SIGINT, die);
	init();		// set up user list and number of ballot items
	short SERV_TCP_PORT = default_port;

#if 0
	if ( argc == 2 ) 
		SERV_TCP_PORT = atoi(argv[1]);
#endif

	// open a TCP socket 
	if ( ( sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 )
	{
		cerr << "voted: can not open stream socket\n";
		exit(1);
	}

	// bind our local address so that the client can send to us 
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons((u_short)SERV_TCP_PORT + portbase);

	if ( verbose )
		cout << "Listening on port " << ntohs(serv_addr.sin_port) << endl;

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) 
	{
		cerr << "voted: can not bind local address\n";
		exit(2);
	}

	listen(sockfd, 5);
	cout << welcome << endl;
	clilen = sizeof(cli_addr);	// no reason to recompute this every time

	while ( 1 )
	{
		// wait for a connection from a client process.  

		current_time = time(NULL);
		if ( current_time >= next_update )
		{
			// it's time, check for new votes
			statefile.open(statefilename);
			statefile >> num_votes;
			statefile.close();
			if ( num_votes >= voting_threshold )
				write_results();
			current_time = time(NULL);
			next_update = current_time + 5;
		}
		if ( verbose )
			cout << "Waiting for incoming connections...\n";
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if( newsockfd < 0)
		{
			cerr << "voted: accept error\n";
			exit(3);
		}

#if FORKING
		if ( ( childpid = fork() ) < 0 )
		{
			cerr << "voted: fork error\n";
			exit(4);
		}
		else
#else
		childpid = 0;
#endif
		if ( childpid == 0 )		 // child process 
		{
#if FORKING
			close(sockfd);				 // close original socket 
#endif

			httpser(newsockfd);			 // process the request 
			if ( verbose )
			{
				cout << "Connection closed.\n";
				cout << "\nWaiting for incoming connections...\n";
			}
#if FORKING
			shutdown();		// clean up dynamic memory (list of users)
			exit(0);
#endif
		}
		//close(newsockfd);

	}

	shutdown();		// clean up dynamic memory (list of users)
	return 0;
}


int getvalue( char c )
{
	int val = 0;
	if ( c >= 'a' && c <= 'z' )
		val = - ( c-'a' );
	else if ( c >= 'A' && c <= 'Z' )
		val = ( c-'A' );
	else
		val = 0;

	return val;
}


/************************************************************************
	Handles all the useful work for a child process.  Is passed the
	active socket descriptor, and accepts the request and serves the
	appropriate file.
*************************************************************************/
void httpser(int sockfd)
{
	int n,i;
	char request[MAXLINE];
	char username[MAXLINE];
	char raw_passwd[MAXLINE];
	char hashed_passwd[33];
	char votestring[MAXLINE];
#if 0
	char filename[MAXLINE], response[MAXLINE];
#endif

	// n = writen(sockfd, welcome, sizeof(char)*strlen(welcome));

	n = readline(sockfd, request, MAXLINE);
	request[n] = 0;               // null terminate
	if ( verbose )
		cout << "Accepted vote from client: " << request << endl;

	i=0;
	while ( request[i] != ' ' )
		username[i] = request[i++];
	username[i++] = 0;			// null terminate username (and skip space)
	n = i;
	while ( request[i] != ' ' )
		raw_passwd[i-n] = request[i++];
	raw_passwd[i++-n] = 0;			// null terminate password (and skip space)

	hash_passwd( raw_passwd, hashed_passwd );

	n = i;
	while ( isalpha( request[i] ) )
		votestring[i-n] = request[i++];
	votestring[i-n] = 0;

	if ( authorized(username, hashed_passwd) )
	{
		tally(votestring);
		writen(sockfd, "ACK\n", 5);
		if ( verbose )
			cout << "voted: accepted vote from " << username << endl;
	}
	else
	{
		writen(sockfd, "NACK\n", 6);
		if ( verbose )
			cout << "voted: rejected vote from " << username << endl;
	}

#if 0
	sscanf(request, "GET /%s", &filename);
	cout << "filename: " << filename << endl;

	cout << "Serving file... ";
	send_file(sockfd, filename);
	cout << filename << " sent.\n";
#endif

	if ( verbose )
		cout << "Closing connection...";
	close(sockfd);
	if ( verbose )
		cout << "transaction complete.\n\n";
}


void init()
{
	int i,n;
	char line[MAXLINE];
	char buf[MAXLINE];
	
	ifstream allow_file, deny_file, ballot_file;
	ofstream state_file;

	// initialize allowed users
	allow_file.open(allowfilename);
	while ( allow_file >> line )
	{
		i=0;
		while ( line[i] != ':' )
			buf[i] = line[i++];
		buf[i++] = 0;		// null terminate username (and skip colon)
		users[num_users].name = new char[strlen(buf)+1];
		strcpy(users[num_users].name,buf);
		
		n = i;
		while ( line[i] != '\0' )
			users[num_users].passwd[i-n] = line[i++];
		users[num_users].passwd[i-n] = 0;
		++num_users;
	}
	allow_file.close();

	// mask out denied users
	deny_file.open(denyfilename);
	while ( deny_file >> buf )
	{
		for ( n=0; n<num_users; ++n )
			if ( strcasecmp(buf,users[n].name) == 0 )
			{
				--num_users;
				delete [] users[n].name;		// remove this entry
				users[n] = users[num_users];	// replace with last
				if ( n != num_users )			// make last one go away
					users[num_users].name = NULL;
				--n;							// and try this slot again
			}
	}
	deny_file.close();

#ifdef DEBUG2
	cout << "Allowed users:\n";
	for ( n=0; n<num_users; ++n )
		cout << users[n].name << endl;
	cout << endl;
#endif

	// initialize voted array to all false
	for ( n=0; n<num_users; ++n )
		already_voted[n] = false;

	// now find count of ballot items
	ballot_file.open(ballotfilename);
	while ( ballot_file.getline(buf,MAXLINE) )
	{
		// we just read in the "human portion" of this item
		n = strlen(buf);
		ballot_names[num_ballot_items] = new char[n+1];
		strcpy(ballot_names[num_ballot_items],buf);

		// now get the action associated with that
		ballot_file.getline(buf,MAXLINE);
		n = strlen(buf);
		ballot_actions[num_ballot_items] = new char[n+1];
		strcpy(ballot_actions[num_ballot_items],buf);
		++num_ballot_items;
	}
	ballot_file.close();
	// and sort them to fix the stupid off-by-one error
	pairsort(ballot_names, ballot_actions, num_ballot_items);

#ifdef DEBUG
	cout << "Number of ballot items: " << num_ballot_items << endl;
	for ( n=0; n<num_ballot_items; ++n )
		cout << ballot_actions[n] << endl;
	cout << endl;
#endif

	// create fresh (empty) voting state file
	state_file.open(statefilename);
	state_file << "0\n";
	for ( n=0; n<num_ballot_items; ++n )
		state_file << "0 ";
	state_file.close();

	// create fresh (empty) results file
	ofstream resultsfile;
	resultsfile.open(resultsfilename);
	resultsfile.close();

}


void shutdown()
{
	int n;
	for ( n=0; n<num_users; ++n )
		delete [] users[n].name;
	for ( n=0; n<num_ballot_items; ++n )
		delete [] ballot_actions[n];
	num_ballot_items = 0;
	num_users = 0;
}


bool authorized( const char * username, const char * passwd )
{
	for ( int n=0; n<num_users; ++n )
		if ( ! already_voted[n]
			&& strcasecmp(users[n].name, username) == 0 
			&& strcmp(users[n].passwd, passwd) == 0 )
		{
			already_voted[n] = true;
			return true;
		}
	return false;
}


void tally( char * votestring )
{
	// This suffers from MAJOR mutex issues, which will need to be
	// resolved before this'll work properly.  Probably can be cleanly (if
	// not portably) fixed by using shared IPC memory in Linux rather than
	// this file.  And maybe a lockfile to enforce mutual exclusion.

	int * values = new int[num_ballot_items];
	int num_votes = 0;
	int i,n;

	// read in current standings
	ifstream statein;
	statein.open(statefilename);
	statein >> num_votes;
	for (i=0; i<num_ballot_items; ++i)
		statein >> values[i];
	statein.close();

	// clamp values if enabled
	if ( clamp_value )
		clamp(votestring, clamp_value);

	// tally current results
	n = strlen(votestring);
	if ( n > num_ballot_items ) n = num_ballot_items;
	for (i=0; i<n; ++i)
		values[i] += getvalue(votestring[i]);

	// write results back to file
	ofstream stateout;
	stateout.open(statefilename);
	stateout << num_votes+1 << '\n';
	for (i=0; i<num_ballot_items; ++i)
		stateout << values[i] << ' ';
	stateout.close();

	// and that's all he wrote (fwiw)
	delete [] values;

	return;
}


void clamp( char* votestring, int threshold )
{
	// Checks for too many votes in either direction, and scales everything
	//  down to the provided threshold

	int * values = new int[num_ballot_items];
	memset(values, 0, num_ballot_items*sizeof(int));

	int i,n,val;
	double yes_total = 0;
	double no_total = 0;
	double yes_scale = 1;
	double no_scale = 1;

	// Find out how much these votes are worth

	n = strlen(votestring);
	if ( n > num_ballot_items ) {
		n = num_ballot_items;
		votestring[n] = '\0';
	}
	for (i=0; i<n; ++i) {
		val = getvalue(votestring[i]);
		if ( val < 0 )
			no_total -= val;
		else
			yes_total += val;
		values[i] = val;
	}

	// Now fix them if they weigh too much
	if ( yes_total > threshold || no_total > threshold ) {
		if ( verbose )
			cout << "Vote too heavy, scaling...";
		yes_scale = (double)threshold / yes_total;
		no_scale = (double)threshold / no_total;
		for (i=0; i<n; ++i) {
			if ( values[i] < 0 ) {
				values[i] = (int) ceil ( values[i] * no_scale );
				votestring[i] = 'a' + char( - values[i] );
			}
			else {
				values[i] = (int) ceil ( values[i] * yes_scale );
				votestring[i] = 'A' + char( values[i] );
			}
		}
		if ( verbose )
			cout << "result is " << votestring << endl;

	}

}



void write_results()
{
	frame * values = new frame[num_ballot_items];
	int num_votes = 0;
	int i;

	// read in current standings
	ifstream statein;
	statein.open(statefilename);
	statein >> num_votes;
	for (i=0; i<num_ballot_items; ++i)
	{
		statein >> values[i].value;
		values[i].pos = i;
	}
	statein.close();

	// now sort the results and write out all positive results to a file
	qsort(values,num_ballot_items,sizeof(frame),framecmp);

	ofstream resultsfile;
	resultsfile.open(resultsfilename);
	i = 0;
	while ( values[i].value > 0 )
	{
		resultsfile << values[i].value << ' '
		            << ballot_actions[ values[i].pos ] << '\n';
		++i;
	}
	resultsfile.close();

	delete [] values;
}


int framecmp(const void* a, const void* b)
{
	frame* af = (frame*)a;
	frame* bf = (frame*)b;
	if ( af->value < bf->value ) return 1;
	if ( af->value > bf->value ) return -1;
	return 0;
}


void pairsort(char ** a, char ** b, int n)
{
	// A and B are associative arrays, sort them based on values in A

	char* temp;
	int i,j;
	for (i=1; i<n; ++i)
		for ( j=n-1; j>=i; --j)
			if ( strcasecmp ( a[j], a[j-1] ) < 0 )
			{
				// swap 'em
				temp = a[j];
				a[j] = a[j-1];
				a[j-1] = temp;
				temp = b[j];
				b[j] = b[j-1];
				b[j-1] = temp;
			}
	
}


void hash_passwd( char* raw, char* hashed )
{
	// Generates the MD5 hash for a given null-terminated ASCII string.
	//  Zeroes out the original password as soon as possible.
	//  'hashed' is assumed to be at least 33 bytes in size.

	const char digit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
	                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	unsigned size = 0;
	unsigned char* digest = NULL;
	MHASH td;
	td = mhash_init(MHASH_MD5);
	if ( td == MHASH_FAILED ) {
		cerr << "Unable to init hash.\n";
		exit(1);
	}

	mhash( td, raw, strlen(raw) );
	memset( raw, 0, strlen(raw) );
	digest = (unsigned char*)mhash_end(td);
	size = mhash_get_block_size(MHASH_MD5);
	for ( unsigned i=0, j=0; i<size; ++i, j+=2 ) {
		hashed[j] = digit[ digest[i] / 16 ];
		hashed[j+1] = digit[ digest[i] % 16 ];
	}
	hashed[size*2] = '\0';

}


void rebuild(int)
{
	if ( verbose )
		cout << "Received interrupt: resetting.\n";
	shutdown();
	config();
	init();
}


void die(int)
{
	if ( verbose )
		cout << "Received interrupt: quitting.\n";
	shutdown();
	unlink("/var/run/voted.pid");		//FIXME: make this configurable
	exit(0);
}


void daemonize()
{
	int pid;
	pid = fork();

	if ( pid > 0 )
		exit(0);
	else if ( pid < 0 )
	{
		cerr << "voted: unable to fork()" << endl;
		exit(1);
	}
}
