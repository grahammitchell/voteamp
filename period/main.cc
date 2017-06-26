// period  - a program to keep track of what period it is
//   mostly done 22 Dec 2000
//   cleaned up a bit 3 Jan 2001

// When first run, you need to tell it what period it currently is.
// It assumes that we are on a common ABABA BABAB block (A and B days simply
// alternate).  The labels for any particular time period are listed in a
// crontab-like file.

// Known holidays can be listed in the same file.
// Weekends are handled because they just simply aren't listed in the file.

#include <iostream.h>
#include <fstream.h>
#include <signal.h>   // signal
#include <unistd.h>	  // fork, unlink
#include <string.h>	  // strcmp
#include <time.h>	  // time, localtime
#include "vector.cpp" // my vector container class
#include "globals.h"  // struct period_ent, debug_print, clear
#include "init.h"     // init

// Some global variables the signal handlers need to get to
bool quit = false;
bool position_changed = false;

int position;                     // slot in the vector we're currently on
int count;                        // how many items are in the vector
int drift;                        // syncs sleep with beginning of minute
vector<period_ent> timelist;      // vector of times

const char* inifile = "/usr/local/etc/periodrc";

// Signal handlers
void shutdown(int);  // just quits
void skip(int);      // moves to next position in the list
void hup(int);       // rereads ini file

// Functions we use to streamline the code
bool time_cmp ( const period_ent & , const struct tm * );
int prompt_for_period( const vector<period_ent> &, int );
void daemonize();


int main(int argc, char* argv[])
{
	if ( argc != 2 )
	{
		cerr << "Usage: period [outfile]\n";
		exit(1);
	}

	signal(SIGINT, shutdown);
	signal(SIGTERM, shutdown);
	signal(SIGHUP, hup);
	signal(SIGUSR1, skip);

	const char* outfile = argv[1];
	ofstream out;
	time_t epoch_secs;
	struct tm *t;

	// initialize vector from "crontab" file
	init(inifile, timelist, count);
	
	// Ask what period it is
	position = prompt_for_period( timelist, count );
	position_changed = true;

	// Daemonize now that we're fairly certain there've been no input errors
	daemonize();

	// We want to syncronize our sleep() with beginning of minute
	drift = 0;

	// Setup is over.
	// Main processing loop.
	while ( ! quit )
	{
		// Get the system time.
		time(&epoch_secs);
		t = localtime(&epoch_secs);
		// Compare the current time to the next time item in the list
		if ( time_cmp( timelist[position], t ) )
			position_changed = true;
		if ( position_changed )
		{
			// Overwrite output file with current label
			out.open(outfile);
			out << timelist[position].label << '\n';
			out.close();
#ifdef VERBOSE
			cerr << "The current time corresponds to label \"";
			cerr << timelist[position].label << "\"\n";
#endif
			// Move our position to the next time item (with wraparound)
			position = (position+1) % count;
			position_changed = false;
		}
		// And take a nap.
		if ( drift > 0 )
			sleep(60);
		else
		{
			// Synchronize our sleep() to wake just after the minute changes.
			// Runs the very first time
			//    and then once an hour to correct for drift.

			// Get the system time.
			time(&epoch_secs);
			// Figure out how many seconds past the minute it is.
			t = localtime(&epoch_secs);
			int secs_this_min = t->tm_sec;
			// 60 secs minus that # of secs tells us how long until the next min
			// Sleep for that number of seconds (+ 1 extra second just in case).
			sleep(60-secs_this_min+1);
			// Set up for the next go around (sixty minutes from now)
			drift = 60;
		}
		--drift;

	}

	unlink(outfile);
	cerr << "period is exiting.\n";

	return 0;
}


// Signal handlers.

void shutdown(int sig)
{
	// Makes the main loop stop
	quit = true;
}


void skip(int sig)
{
	// No need to increment 'position' since it's already one past
	position_changed = true;
	// Handling this signal stops sleeping, so we need to resync our sleep()
	drift = 0; 
}


void hup(int sig)
{
	// Re-reads ini file, resets position to beginning of list
	init(inifile, timelist, count);
	position = 0;
	position_changed = true;
	// Handling this signal stops sleeping, so we need to resync our sleep()
	drift = 0;
}


bool time_cmp ( const period_ent & p, const struct tm * t )
{
	return ( ( p.min[ t->tm_min ] )
	  && ( p.hr[ t->tm_hour ] )
	  && ( p.monthday[ t->tm_mday-1 ] )
	  && ( p.month[ t->tm_mon ] )
	  && ( p.weekday[ t->tm_wday ] )
	);
}


void daemonize()
{
	int pid;
	pid = fork();

	if ( pid > 0 )
		exit(0);
	else if ( pid < 0 )
	{
		cerr << "period: unable to fork()" << endl;
		exit(1);
	}
}


int prompt_for_period( const vector<period_ent> & timelist, int count )
{
	// Ask what period it is
	// (The only reason we need to know this is so we'll know
	//    if it's an A day or a B day.)
	char current_label[80];
	int position;                     // slot in the vector we're currently on
	int i;

	// Print a list of the known labels so they know what their choices are.
	for ( i=0; i<count; ++i )
		cout << '[' << timelist[i].label << ']' << endl;
	cout << "What period is it now? ";
	cin >> current_label;

	// Find their label in the vector of times
	for ( i=0; i<count; ++i )
		if ( strcmp( timelist[i].label, current_label ) == 0 )
			position = i;

	if ( position == -1 )
	{
		// They typed a label we don't have in the timelist
		cerr << "period: unable to locate label " << current_label << ":\n";
		cerr << "Candidates are:\n";
		for ( i=0; i<count; ++i )
			cerr << '\t' << timelist[i].label << endl;
		exit(1);
	}

	return position;
}

