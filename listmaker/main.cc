// listmaker 0.1.4 (c) 2003 Graham Mitchell <Graham.Mitchell@leanderisd.org>
//        24 Jul 1999
//         3 Jan 2001 updated to check blacklist database
//         9 Feb 2001 fixed to really handle ties (didn't really do this)
//         7 Mar 2002 don't try to use database if called with --no_db
//		2003-09-03 created startup script for init.d,
//		            now writes pid to /var/run/listmaker.pid
//		            bugfixes and cleanup
//		2003-09-04 trying to fix quiet failure after the program has been
//		            running longer than a minute or so.  Suspect it's because
//		            'infile' and 'outfile' were just pointers to argv values,
//		            which are probably violatile after a fork().
//		           added error logging to /var/log/listmaker.log
//		           moving the ifstream variables inside their loops so they
//		             get created and destroyed each time fixes the problem,
//		             but it shouldn't.


// main.cc
// Main program

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

#include <iostream>
#include <fstream>
#include <unistd.h>  // fork(), unlink()
#include <stdlib.h>  // atoi(), something else?
#include <stdio.h>   // sprintf()
#include <signal.h>
#include <sys/stat.h>
#include <cstring>
#include <time.h>
#include <mysql/mysql.h>

const int MAXLINE = 1024;
bool lucky( int );
void reduce( int& );
void shutdown(int);
void rebuild_handler(int);
void daemonize();
bool blacklisted( char* track, char* period, unsigned long timestamp );
void try_mysql_query( MYSQL*, char* );

bool done = false;
bool rebuild = false;
bool use_db = true;

const unsigned long timeout = 10 * 24* 60 * 60;	// ten days of seconds

std::ofstream errlog;

MYSQL mysql;

int main(int argc, char **argv)
{
	srand(unsigned(time(NULL)));

	signal(SIGINT,shutdown);
	signal(SIGTERM,shutdown);
	signal(SIGHUP,rebuild_handler);

	// okay, this isn't going to be particularly great

	// what we're going to do is open the first file and randomly select
	// songs, avoiding repeats, until 2/3 of the songs have been chosen
	// Add those songs to a big array

	// Then open the second file and randomly select songs until 1/3 of the
	// songs have been chosen.  Add these also.

	// Next file, 1/6, 1/12 and so on until the percentage is smaller than
	// the number of songs available (i.e. 1/12 songs to be picked but only
	// ten tracks).  Then you have a 50% chance of picking a song from that
	// album, then a 25% chance, and so on, but never dropping below 10%

	// Once all the possible songlist items have been picked through, the
	// list is randomly shuffled and written to disk, possibly making sure
	// that the first song is from the first album.

	// On second thought, implementation is much easier if we just do a
	// percentage chance of each song making it on the final playlist, starting 
	// at %66 percent, and halving each album, but never dropping below 1%
	// (assumes ten songs per album, so 1% chance for each song means a %10
	// chance that some song from that album will make it on)

	std::ofstream output_file;

	const char* periodfile = "/var/run/current_period";
	const char* errlogfile = "/var/log/listmaker.log";
	char period[20] = { 0 };
	std::ifstream period_in;

	struct stat statbuf;
	unsigned long checksum = 0;
	unsigned long newcheck = 0;
	unsigned long timestamp = 0;

	char* playlist[256];	// FIXME: resultant playlist has max 256 tracks
	char current_list[MAXLINE]  = { 0 };
	char current_track[MAXLINE] = { 0 };
	char current_path[MAXLINE]  = { 0 };
	char infile[MAXLINE] = { 0 };
	char outfile[MAXLINE] = { 0 };
	int i=0, len, pathlen;
	int percent = 66;

	if ( argc < 3 )
	{
		std::cerr << "Usage: " << argv[0] << " [metalist] [outputlist]\n";
		exit(1);
	}

	// parse command-line parameters
	for ( i=1; i<argc; ++i )
	{
		if ( strcmp( argv[i], "--no_db" ) == 0 )
			use_db = false;
		else if ( infile[0] == '\0' )
			strcpy(infile,argv[i]);
		else
			strcpy(outfile,argv[i]);
	}

	if ( infile[0] && outfile[0] )
		daemonize();
	else
	{
		std::cerr << "Usage: " << argv[0] << " [metalist] [outputlist]\n";
		exit(1);
	}

	errlog.open(errlogfile);
	if ( ! errlog.good() )
	{
		std::cerr << "Failed to open error log \"" << errlogfile << "\"\n";
		exit(1);
	}

	if ( use_db )
	{
		// Init mysql object and connect to mySQL database
		mysql_init(&mysql);
		if ( ! mysql_real_connect(&mysql,"localhost","voter","songselect","song_blacklist",0,NULL,0))
		{
			std::cerr << "listmaker failed to connect to database: Error: ";
			std::cerr << mysql_error(&mysql) << '\n';
			errlog << "listmaker failed to connect to database: Error: ";
			errlog << mysql_error(&mysql) << '\n';
			exit(1);
		}
	}

	// Main polling loop.
	do
	{
		std::ifstream metafile;

		// check status of file
		stat(infile, &statbuf);
		newcheck = (unsigned long) statbuf.st_mtime;

		if ( newcheck != checksum )	// input file has been changed, rebuild
			rebuild = true;
		if ( rebuild )
		{
			// First off, what period is it?
			period_in.open(periodfile);
			period_in >> period;
			period_in.close();
			if ( ! period[0] ) // something wrong, maybe file doesn't exist
				strcpy(period,"none");

			// And what time is it?
			timestamp = time(NULL);

			metafile.open(infile);
			if ( ! metafile.good() )
			{
				std::cerr << "Failed to open metafile \"" << infile << "\"\n";
				errlog << "Failed to open metafile \"" << infile << "\"\n";
				exit(1);
			}

			percent = 66;
			i=0;
			// Loop for each album
			while ( metafile >> current_list )
			{
				std::ifstream current_file;
#ifdef DEBUG
				std::cout << "****" << current_list << ':' << percent << '\n';
#endif
				len = strlen(current_list) - 1;
				// Remove "album.m3u" from end to get path
				while ( current_list[len] != '/' )
					--len;
				memset(current_path,0,MAXLINE);
				strncpy(current_path,current_list,len+1);
				pathlen = strlen(current_path);
				// Open current list of tracks
				current_file.open(current_list);
				if ( ! current_file.good() )
				{
					std::cerr << "Failed to open file \"" << current_list << "\"\n";
					errlog << "Failed to open file \"" << current_list << "\"\n";
					exit(1);
				}
				memset(current_track,0,MAXLINE);
				// Loop for each track
				while ( current_file >> current_track )
				{
					// Got a track name
					if ( lucky(percent) )
					{
						// The odds say to play this one.

						// Prepend path to track name
						len = strlen(current_track) + pathlen;
						playlist[i] = new char[len+1];
						strcpy(playlist[i],current_path);
						strcat(playlist[i],current_track);

						// Now check to see if it's not blacklisted
						if ( ! blacklisted( playlist[i], period, timestamp ) )
						{
							// We're not blacklisted; add this track
#ifdef DEBUG
							std::cout << '+' << current_track << '\n';
#endif
							// The name is already in the right place, so all
							// we have to do is move to the next slot in the
							// list.
							++i;
						}
						else
						{
							// don't add this track because it's blacklisted
							delete [] playlist[i];
						}
							
					}
#ifdef DEBUG
					else
					{
						// don't add this track because it's unlucky
						std::cout << '-' << current_track << '\n';
					}
#endif
				}
				current_file.close();
				// For the next album, the odds aren't as good
				reduce(percent);
			}
			metafile.close();

			// At this point, playlist should have all the items and 'i' should
			// be the number of tracks (not the index of the last item)

			// Shuffle the thing (using pointers to avoid useless strcpy)

			int a, b;
			char* temp;
			for ( len=0; len<i*20; ++len )
			{
				// pick two random slots, exchange them
				a = rand()%i;
				b = rand()%i;
				temp = playlist[a];
				playlist[a] = playlist[b];
				playlist[b] = temp;
			}

			// And finally write it out to the output file

			output_file.open(outfile);
			for ( len=0; len<i; ++len )
				output_file << playlist[len] << '\n';
			output_file.close();

			// now clean up and go home

			for ( len=0; len<i; ++len )
				delete [] playlist[len];

			checksum = newcheck;
			rebuild = false;
		}
		sleep(10);

	} while ( !done );

	// Close connection to database
	if ( use_db )
		mysql_close(&mysql);

	unlink("/var/run/listmaker.pid");
	std::cerr << "listmaker exiting....";
	errlog.close();

	return 0;
}


bool lucky ( int percent )
{
	return ( (rand()%100+1) <= percent );
}


void reduce ( int& val )
{
	val /= 2;
	if ( val == 0 ) val = 1;
}


void daemonize()
{
	int pid;
	pid = fork();

	if ( pid > 0 )
	{
		// parent process; write child's pid to /var/run/listmaker.pid
		std::ofstream file;
		file.open( "/var/run/listmaker.pid" );
		if ( ! file.good() )
		{
			std::cerr << "listmaker: couldn't write pid to /var/run/listmaker.pid\n";
			exit(1);
		}
		file << pid << '\n';
		file.close();
		// then close
		exit(0);
	}
	else if ( pid < 0 )
	{
		std::cerr << "listmaker: unable to fork()\n";
		exit(1);
	}
}

// Signal handlers.

void shutdown(int sig)
{
	done = true;
}

void rebuild_handler(int sig)
{
	rebuild = true;
}


bool blacklisted( char* track, char* period, unsigned long now )
{

	if ( ! use_db )
		return false;

	MYSQL_ROW row;
	MYSQL_RES *result;

	unsigned long expires;    // time after which the track should leave the db
	unsigned long insertion;  // time when the track was inserted in the db

	char quoted_track[MAXLINE]  = { 0 };
	char quoted_period[MAXLINE]  = { 0 };

	// Escape quotes and such
	mysql_real_escape_string(&mysql,quoted_track,track,strlen(track));
	mysql_real_escape_string(&mysql,quoted_period,period,strlen(period));

	// Sprintf the track name and period into a SELECT query
	//       to retrieve the desired insertion time (if any match)
	char query_string[MAXLINE] = { 0 };
	sprintf(query_string,
		"SELECT expires FROM track_list WHERE track = '%s' AND period = '%s'",
		quoted_track,
		quoted_period
	);

	// Submit the query
	try_mysql_query(&mysql, query_string);

	// Retrieve insertion time (if any match)
	result = mysql_store_result(&mysql);
	while ( (row = mysql_fetch_row(result)) )
	{
		// Convert the insertion time to an integer
		insertion = atoi(row[0]);
		// Add timeout to get expiration time
		expires = insertion + timeout;

		// Compare the expiration time with the current time
		if ( expires >= now )  // "if it expires in the future"
		{
			// This one is still blacklisted.  Clean up and go.
			mysql_free_result(result);
			return true;
		}
		else
		{
			// The track is listed, but its blacklist period is over.

			// Make a query string to prune it from the blacklist
			sprintf(query_string,
				"DELETE FROM track_list WHERE track = '%s' AND period = '%s' AND expires = '%lud'",
				quoted_track,
				quoted_period,
				insertion
			);

			// Submit the query
			try_mysql_query(&mysql, query_string);

		}
	}
	mysql_free_result(result);

	// If we got this far, we found no tracks that were blacklisted.
	return false;

}


void try_mysql_query( MYSQL* mysql_p, char* query_string )
{
	if ( mysql_query(mysql_p, query_string) )
	{
		std::cerr << "mySQL query failed\n";
		std::cerr << '\t' << query_string << '\n';
		std::cerr << '\t' << mysql_error(&mysql) << '\n';
		errlog << "mySQL query failed\n";
		errlog << '\t' << query_string << '\n';
		errlog << '\t' << mysql_error(&mysql) << '\n';
		exit(1);
	}
}
