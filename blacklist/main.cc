// blacklist - monitor currently-playing song.  Add the song, with current
//             period and timestamp to a blacklist database so the song won't
//             be played again for five days or so.

// started 2 Jan 2001
// finished 3 Jan 2001

#include <sys/stat.h>
#include <time.h>    // time()
#include <unistd.h>  // sleep(), fork()
#include <stdlib.h>  // exit()
#include <stdio.h>   // sprintf()
#include <string.h>   // strcpy(), strlen()
#include <signal.h>  // signal()
#include <iostream.h>
#include <fstream.h>
#include <mysql/mysql.h>

void shutdown(int);
void toggle_ignore(int);
void daemonize();
void add_to_blacklist(char* , char * , unsigned long );

bool done = false;
bool ignore = false;
MYSQL mysql;

int main()
{
	signal(SIGINT,shutdown);
	signal(SIGHUP,shutdown);
	signal(SIGTERM,shutdown);
	signal(SIGUSR1,toggle_ignore);

	daemonize();

	// Init mysql object
	mysql_init(&mysql);

	// Connect to mySQL database
	if ( !  mysql_real_connect(&mysql,"localhost","voter","songselect","song_blacklist",0,NULL,0))
	{
		cerr << "Failed to connect to database: Error: ";
		cerr << mysql_error(&mysql) << endl;
		exit(1);
	}

	bool infile_changed = true;

	const char* songfile = "/var/run/now_playing";
	char track[255] = { 0 };
	ifstream track_in;

	const char* periodfile = "/var/run/current_period";
	char period[20] = { 0 };
	ifstream period_in;

	struct stat statbuf;
	unsigned long checksum = 0;
	unsigned long newcheck = 0;
	unsigned long timestamp = 0;

	// Main loop.
	do
	{
		if ( ! ignore )
		{
			// check status of file
			stat(songfile, &statbuf);
			newcheck = (unsigned long) statbuf.st_mtime;
			if ( newcheck != checksum )
				infile_changed = true;
			if ( infile_changed )
			{
				// File has changed, get new track info

				// Figure out current period
				period_in.open(periodfile);
				period_in >> period;
				period_in.close();
				if ( ! period[0] ) // something wrong, maybe file doesn't exist
					strcpy(period,"none");

				// Figure out current song
				track_in.open(songfile);
				track_in >> track;
				track_in.close();

				// Figure out current time
				timestamp = time(NULL);
				
				// Now add this track to the blacklist database
				if ( track[0] )
					add_to_blacklist(period, track, timestamp);

				// And set things up for the next go around
				checksum = newcheck;
				infile_changed = false;
			}
		}
		sleep(5);

	} while ( !done );

	mysql_close(&mysql);
	cerr << "blacklist exiting....\n";

	return 0;
}


void daemonize()
{
	int pid;
	pid = fork();

	if ( pid > 0 )
		exit(0);
	else if ( pid < 0 )
	{
		cerr << "blacklist: unable to fork()" << endl;
		exit(1);
	}
}


void add_to_blacklist(char* raw_period, char * raw_track, unsigned long timestamp)
{
	// Escape quotes and such
	char period[40];
	char track[512];
	mysql_real_escape_string(&mysql,period,raw_period,strlen(raw_period));
	mysql_real_escape_string(&mysql,track,raw_track,strlen(raw_track));

	// Sprintf everything into a query string
	char query_string[600] = { 0 };
	sprintf(query_string,
		"INSERT INTO track_list VALUES ('%s', '%s', '%lud')", 
		period,
		track,
		timestamp
	);

	// Query to insert it into the database
	if ( mysql_query(&mysql, query_string) )
	{
		cerr << "Insert failed: " << mysql_error(&mysql) << endl;
		exit(1);
	}

}


void toggle_ignore(int n)
{
	ignore = ! ignore;
}


void shutdown(int n)
{
	done = true;
}
