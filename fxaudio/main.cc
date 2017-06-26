// fxaudio 0.1.0 (c) 1999 Graham Mitchell <mitchell@cs.leander.isd.tenet.edu>
//     24 Jul 99

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

#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>

const int MAXLINE = 1024;
int DEST = 0;

bool quit = false;
bool done = false;
bool now_playing = false;
bool detach = false;
bool paused = false;
bool verbose = false;
int current_track;
int max_track;
int num_tracks = 0;
unsigned long checksum = 0;
char listname[MAXLINE] = { 0 };
char now_playing_buf[MAXLINE] = { 0 };
struct stat statbuf;
char* playlist[256]; 	// FIXME: Remove hard-coded limit

char* openit = NULL;
const char* playit = "play\n";
const char* closeit = "close\n";

void player(int, int);
void forker();
void output_now_playing( char * );
bool check_playlist();	// Rebuilds playlist if changed
char* opentrack(int);	// Constructs 'open' command for given song
void handler(int);		// catch kill signal to clean up
void skip(int);			// signal handler to jump to next track
void pauser(int);		// signal handler to pause


int main(int argc, char* argv[])
{
	// Initialize songlist from ini file
	//	then call forker(), which sets up the wrapper file-descriptors and
	//		spawns rxaudio
	//	then forker() will call player(), which actually plays the songs

	int len;
	ifstream ini;
	signal(SIGTERM, handler);
	signal(SIGHUP, handler);
	signal(SIGINT, handler);
	signal(SIGUSR1, skip);
	signal(SIGUSR2, pauser);
	
	// parse command-line options, find filename and -quit
	for (len=1; len<argc; ++len)
	{
		if ( argv[len][0] == '-' && strcasecmp(argv[len],"-quit") == 0 )
			quit = true;
		else if ( argv[len][0] == '-' && argv[len][1] == 'v' )
			verbose = true;
		else if ( argv[len][0] == '-' && strcasecmp(argv[len],"-detach") == 0 )
			detach = true;
		else
		{
			if ( listname[0] == 0 )
				strcpy(listname, argv[len]);
			else {
				strcpy(now_playing_buf, argv[len]);
				now_playing = true;
			}
		}
				
	}
	if ( listname[0] == 0 )
		strcpy(listname, "album.m3u");
		
	ini.open(listname);
	if ( ini.fail() )
	{
		cerr << "fxaudio: file not found (" << listname << ")\n";
		return 1;
	}

	stat(listname, &statbuf);
	checksum = (unsigned long) statbuf.st_mtime;

	char buf[MAXLINE] = { 0 };		// used to hold filenames temporarily
	
	current_track = 0;
	while ( ini >> buf )
	{
		len = strlen(buf);
		playlist[current_track] = new char[len+1];
		strcpy(playlist[current_track], buf);
		++current_track;
	}
	ini.close();
	max_track = current_track - 1;
	num_tracks = current_track;
	current_track = 0;

	// Let's detach ourselves if that's what's wanted
	if ( detach ) {
		int pid;
		pid = fork();

		if ( pid > 0 )
			exit(0);
		else if ( pid < 0 ) {
			cerr << "fxaudio: unable to fork." << endl;
			exit(1);
		}
		// and that should do it.
	}

	// Okay, we should be set up; let's spawn rxaudio and such.

	forker();

	// Now that we're back, recycle the playlist

	for ( current_track=0; current_track<=max_track; ++current_track )
		delete [] playlist[current_track];

	return 0;
}



bool check_playlist()
{
	// check status of playlist, and rebuild if necessary
	unsigned long newcheck = 0;

	newcheck = (unsigned long) statbuf.st_mtime;
	if ( newcheck != checksum )
	{
		// file has been changed, rebuild
		if ( verbose )
		{
			cout << "fxaudio: playlist \"" << listname << "\" altered; rebuilding...";
			cout.flush();
		}

		for ( current_track=0; current_track<=max_track; ++current_track )
			delete [] playlist[current_track];

		ifstream ini;
		ini.open(listname);
		int len = 0;
		checksum = newcheck;

		char buf[MAXLINE] = { 0 };		// used to hold filenames temporarily
		
		current_track = 0;
		while ( ini >> buf )
		{
			len = strlen(buf);
			playlist[current_track] = new char[len+1];
			strcpy(playlist[current_track], buf);
			++current_track;
		}
		ini.close();
		max_track = current_track - 1;
		num_tracks = current_track;
		current_track = 0;
		if ( verbose )
			cout << "done." << endl;
		return ( num_tracks );
	}
	return false;
}

		


void forker()
{
	const int WRITE=1;
	const int READ=0;

	int fd_array_par[2];
	pipe(fd_array_par);

	int fd_array_kid[2];
	pipe(fd_array_kid);

	if ( fork() )
	{
		// parent
		close(fd_array_kid[WRITE]);
		close(fd_array_par[READ]);
	}
	else
	{
		// child
		close(fd_array_par[WRITE]);
		dup2(fd_array_par[READ], READ);
		close(fd_array_par[READ]);

		close(fd_array_kid[READ]);
		dup2(fd_array_kid[WRITE], WRITE);
		close(fd_array_kid[WRITE]);

		execlp("rxaudio","rxaudio",NULL);
	}

	// Since the child is exec'd away, I'm the parent
	player(fd_array_kid[READ], fd_array_par[WRITE]);
}


char* opentrack(int track_num)
{
	// construct 'open' command for given playlist item

	char* cmd = new char[MAXLINE];

	memset(cmd, 0, MAXLINE);
	strcpy(cmd, "open ");
	strcat(cmd, playlist[track_num]);
	strcat(cmd, "\n");

	return cmd;
}


void player(int SRC, int DST)
{
	DEST = DST;		// so the signal handler can see the pipe
	char readbuf[MAXLINE];
	bool playlist_changed = false;
	int size = 0;
	time_t next_check = 0;
	time_t current_time = 0;
	int buf_size = sizeof(char)*MAXLINE;
#if 0
	int choice = 0;
	char writebuf[256];
	const char* volume = "volume 50 100 50\n";
	const char* pauseit = "pause\n";
	const char* stopit = "stop\n";
	const char* quitit = "stop\nclose\nexit\n";
#endif

	size = read(SRC, readbuf, buf_size);
	readbuf[size] = 0;

	// open the first track and begin playing
	if ( num_tracks )
	{
		delete [] openit;
		openit = opentrack(current_track);
		write(DST, openit, sizeof(char)*(strlen(openit)));
		write(DST, playit, sizeof(char)*(strlen(playit)));

		// get status from xaudio (again)
		size = read(SRC, readbuf, buf_size);
		readbuf[size] = 0;

		if ( verbose )
			cout << "fxaudio: playing " << playlist[current_track] << endl;
		output_now_playing( playlist[current_track] );
	}
	else if ( quit == false )
	{
		if ( verbose )
			cout << "fxaudio: waiting patiently for non-empty playlist\n";
		done = true;
	}
	else
	{
		if ( verbose )
			cout << "fxaudio: playlist is empty\n";
		output_now_playing( NULL );
		done = true;
	}
	do {
		if ( done ) sleep(5);
		current_time = time(NULL);
		if ( current_time >= next_check )
		{
			//cout << "fxaudio: checking status of file" << endl;
			stat(listname, &statbuf);
			playlist_changed |= check_playlist();
			current_time = time(NULL);
			next_check = current_time + 10;
		}
		if ( done && playlist_changed )
		{
			// let us begin
			delete [] openit;
			openit = opentrack(current_track);
			write(DST, openit, sizeof(char)*(strlen(openit)));
			write(DST, playit, sizeof(char)*(strlen(playit)));
			if ( verbose )
				cout << "fxaudio: playing " << playlist[current_track] << endl;
			output_now_playing( playlist[current_track] );
			playlist_changed = false;
			done = false;
		}
		if ( ! done )
		{
			size = read(SRC, readbuf, buf_size);
			readbuf[size] = 0;
			if ( strstr(readbuf, "EOF") )
			{
				// time to move on to the next track
				write(DST, closeit, sizeof(char)*(strlen(closeit)));
				if ( ! playlist_changed )
					++current_track;
				if ( current_track > max_track )
				{
					done = true;
					if ( verbose )
						cout << "fxaudio: stopped\n";
					output_now_playing( NULL );
				}
				else
				{
					delete [] openit;
					openit = opentrack(current_track);
					write(DST, openit, sizeof(char)*(strlen(openit)));
					write(DST, playit, sizeof(char)*(strlen(playit)));
					if ( verbose )
						cout << "fxaudio: playing " << playlist[current_track] << endl;
					output_now_playing( playlist[current_track] );
					playlist_changed = false;
				}
			}
		}

	} while ( ! done || ! quit );

	write(DST, "exit\n", 5);
	output_now_playing( NULL );

}


void handler(int)
{
	const char* quitit = "stop\nclose\nexit\n";

	for ( current_track=0; current_track<=max_track; ++current_track )
		delete [] playlist[current_track];
	
	write(DEST, quitit, sizeof(char)*(strlen(quitit)));
	output_now_playing( NULL );
	exit(0);
}


void pauser(int)
{
	const char* pauseon = "pause\n";
	const char* pauseoff = "play\n";

	if ( paused ) {
		write(DEST, pauseoff, sizeof(char)*(strlen(pauseoff)));
		if ( verbose ) cout << "fxaudio: pause off\n";
	}
	else {
		write(DEST, pauseon, sizeof(char)*(strlen(pauseon)));
		if ( verbose ) cout << "fxaudio: paused\n";
	}

	paused = ! paused;
}


void skip(int)
{
	if ( verbose ) cout << "fxaudio: received USR1 (skip)\n";
	if ( done ) return;

	const char* quitit = "stop\nclose\n";

	write(DEST, quitit, sizeof(char)*(strlen(quitit)));
	++current_track;
	if ( current_track > max_track )
	{
		done = true;
		if ( verbose )
			cout << "fxaudio: stopped\n";
		output_now_playing( NULL );
	}
	else
	{
		delete [] openit;
		openit = opentrack(current_track);
		write(DEST, openit, sizeof(char)*(strlen(openit)));
		write(DEST, playit, sizeof(char)*(strlen(playit)));
		if ( verbose )
			cout << "fxaudio: playing " << playlist[current_track] << endl;
		output_now_playing( playlist[current_track] );
	}
}


void output_now_playing( char * trackname )
{
	if ( now_playing )
	{
		ofstream file;
		file.open(now_playing_buf);
		if ( trackname )
			file << trackname << endl;
		file.close();
	}
}
