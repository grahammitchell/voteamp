#include <stdio.h>
#include <fstream.h>
#include <string.h>

#include <pwd.h>		// for getpwuid()
#include <unistd.h>		// for getuid()
#include <sys/types.h>	// for getuid()
#include <curses.h>		// for printw()

#if 0
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "net.h"
#include "ui.h"
#include "globals.h"

// These variables are from globals.h
char* ballot_names[200];		// FIXME: remove hard-coded limit
char* login_name = NULL;
char* password = NULL;
int* votes = NULL;
int num_ballot_items = 0;
// end of variables from globals.h

char* generate_vote();			// creates vote string
char* submit(const char*, int, const char*);	// submits vote to server 
void init();				// initialization & other setup

int main(int argc, char* argv[])
{
	char* ack = NULL;
	char* vote = NULL;

	init();		// read in ballot items from file (and get name & passwd)
	ui_init();

	do_ui();
	vote = generate_vote();

	printw("Submitting vote for %s...", login_name);
	refresh();
	ack = submit("127.0.0.1", 4417, vote);
	printw("%s\n\n",ack);
	refresh();

	return 0;
}	


char* generate_vote()
{
	int i;
	char* ballot, *vote;

	ballot = new char[num_ballot_items+1];
	vote   = new char[num_ballot_items+32+strlen(login_name)+3];

	for ( i=0; i<num_ballot_items; ++i )
	{
		if ( votes[i] > 0 )
			ballot[i] = 'A' + votes[i];
		else if ( votes[i] < 0 )
			ballot[i] = 'a' - votes[i];
		else
			ballot[i] = 'A';
	}
	ballot[num_ballot_items] = '\0';
	sprintf(vote, "%s %s %s", login_name, password, ballot);

	return vote;
}


void init()
{
	char buf[MAXLINE] = { 0 };
	int n;

	ifstream ballot_file;
	ballot_file.open("songs.bal");
	if ( ballot_file.fail() ) {
		cerr << "voter: cannot open ballot file: songs.bal\n";
		exit(1);
	}

	// now input ballot items
	num_ballot_items = 0;
	while ( ballot_file.getline(buf,MAXLINE) ) {
		// we just read in the "human portion" of this item
		n = strlen(buf);
		ballot_names[num_ballot_items] = new char[n+1];
		strcpy(ballot_names[num_ballot_items],buf);

		// now get the action associated with that, and throw it away
		ballot_file.getline(buf,MAXLINE);

		++num_ballot_items;
	}
	ballot_file.close();

	// set up array of votes and initialize to zero
	votes = new int[num_ballot_items];
	for ( n=0; n<num_ballot_items; ++n )
		votes[n] = 0;

	// get login name
	struct passwd *pw;
	pw = getpwuid( getuid() );
	char* tempname = pw->pw_name;
	n = strlen(tempname);
	login_name = new char[n+1];
	strcpy( login_name, tempname );

	// prompt for password
	cout << "Password: ";
	cin >> buf;
	password = new char[strlen(buf)+1];
	strcpy( password, buf );

}


char* submit(const char* server, int port, const char* vote)
{
	int sockfd;
	char* response = new char[6];
	memset(response, 0, 6);

	sockfd = open_socket(server, port);
	writen(sockfd, vote, strlen(vote));
#if 0
	readn(sockfd, response, 5);
#endif
	close(sockfd);

	return response;
}

