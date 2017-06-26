// voteserver 0.1.0 (c) 1999 Graham Mitchell - 24 Jul 99

// init.cc
// Sets global variables from initialization file and overrides
//   file-specified values with command-line parameters.

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

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"

const char* inifilename = "voted.ini";

void process_args(int , char** );
char* strdup( const char* );

void config(int argc, char** argv)
{
	// open configuration file and set values
	char buf[MAXLINE];
	int newargc = 0;
	int len;
	char * newargv[64];		//FIXME: remove hard-coded limit

	ifstream infile;
	infile.open ( inifilename );
	if ( infile.fail() )
		return;

	while ( infile.getline(buf,MAXLINE) )
	{
		if ( buf[0] == '#' || buf[0] == 0)
			continue;
		len = strlen(buf);
		newargv[newargc] = new char[len+1];
		strcpy(newargv[newargc], buf);
		++newargc;
	}
	infile.close();

	// set up defaults (if not already assigned)
	if ( default_port == -1 )
	{
		default_port = 4417;
		verbose = false;
		detach = true;
		allowfilename = strdup( "users.allow" );  // this is a custom strdup
		denyfilename = strdup( "users.deny" );
		statefilename = strdup( "voted.state" );
		ballotfilename = strdup( "voted.ballot" );
		resultsfilename = strdup( "voted.results" );
		auth_mode = 0;
		voting_frequency = -1;
		voting_threshold = 10;
		voting_maximum = 0;
		voting_duration = 0;
		clamp_value = 0;
	}
	else
		argc = 0;

	process_args(newargc,newargv);
	if ( argc )
		process_args(argc-1,&argv[1]);
#ifdef DEBUG
	cout << "default_port = " << default_port << endl;
	cout << "quiet = " << ( verbose ? "OFF" : "ON" ) << endl;
	cout << "foreground = " << ( detach ? "OFF" : "ON" ) << endl;
	cout << "allow_file = " << allowfilename << endl;
	cout << "deny_file = " << denyfilename << endl;
	cout << "auth_mode = " << auth_mode << endl;
	cout << "voting_frequency = " << voting_frequency << endl;
	cout << "state_file = " << statefilename << endl;
	cout << "voting_threshold = " << voting_threshold << endl;
	cout << "voting_maximum = " << voting_maximum << endl;
	cout << "voting_duration = " << voting_duration << endl;
	cout << "ballot_file = " << ballotfilename << endl;
	cout << "results_file = " << resultsfilename << endl;
	cout << "clamp_value = " << clamp_value << endl;
#endif
	return;
}



void process_args(int argc, char** argv)
{
	int i,len;
	char buf[MAXLINE];
	for ( i=0; i<argc; ++i )
	{
		if ( argv[i][0] == '-' )
			strcpy( buf, &argv[i][1]);
		else
			strcpy( buf, argv[i]);

		if ( strncasecmp( buf, "default_port=", 13 ) == 0 )
		{
			default_port = atoi( &buf[13] );
		}
		else if ( strncasecmp( buf, "quiet=", 6 ) == 0 )
		{
			verbose = strcasecmp ( &buf[6], "OFF" ) == 0;
		}
		else if ( ( strncasecmp( buf, "-verbose", 7 ) == 0 )
					|| ( strcmp( buf, "v" ) == 0 ) )
		{
			verbose = true;
		}
		else if (  ( strncasecmp( buf, "-foreground", 11 ) == 0 )
					|| ( strncasecmp( buf, "fg", 2 ) == 0 ) )
		{
			detach = false;
		}
		else if ( strncasecmp( buf, "allow_file=", 11 ) == 0 )
		{
			len = strlen( &buf[11] );
			delete [] allowfilename;
			allowfilename = new char[len+1];
			strcpy( allowfilename, &buf[11] );
		}
		else if ( strncasecmp( buf, "deny_file=", 10 ) == 0 )
		{
			len = strlen( &buf[10] );
			delete [] denyfilename;
			denyfilename = new char[len+1];
			strcpy( denyfilename, &buf[10] );
		}
		else if ( strncasecmp( buf, "state_file=", 11 ) == 0 )
		{
			len = strlen( &buf[11] );
			delete [] statefilename;
			statefilename = new char[len+1];
			strcpy( statefilename, &buf[11] );
		}
		else if ( strncasecmp( buf, "ballot_file=", 12 ) == 0 )
		{
			len = strlen( &buf[12] );
			delete [] ballotfilename;
			ballotfilename = new char[len+1];
			strcpy( ballotfilename, &buf[12] );
		}
		else if ( strncasecmp( buf, "results_file=", 13 ) == 0 )
		{
			len = strlen( &buf[13] );
			delete [] resultsfilename;
			resultsfilename = new char[len+1];
			strcpy( resultsfilename, &buf[13] );
		}
		else if ( strncasecmp( buf, "auth_mode=", 10 ) == 0 )
		{
			auth_mode = atoi( &buf[10] );
		}
		else if ( strncasecmp( buf, "voting_frequency=", 17 ) == 0 )
		{
			voting_frequency = atoi( &buf[17] );
		}
		else if ( strncasecmp( buf, "voting_threshold=", 17 ) == 0 )
		{
			voting_threshold = atoi( &buf[17] );
		}
		else if ( strncasecmp( buf, "voting_maximum=", 15 ) == 0 )
		{
			voting_maximum = atoi( &buf[15] );
		}
		else if ( strncasecmp( buf, "voting_duration=", 16 ) == 0 )
		{
			voting_duration = atoi( &buf[16] );
		}
		else if ( strncasecmp( buf, "clamp_value=", 12 ) == 0 )
		{
			clamp_value = atoi( &buf[12] );
		}
		else
		{
			cerr << "Unrecognized option: " << buf << endl;
		}
	}
}


char* strdup( const char* s )
{
	// custom version of strdup; should be fairly identical except that
	// it uses 'new' rather than 'free' (so delete-ing them is cleaner).

	int n = strlen(s);

	char* t = new char[n+1];
	char* r = t;

	while ( (*t++ = *s++) )
		;
	
	return r;
}
