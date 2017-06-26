#include <iostream.h>
#include <fstream.h>
#include <stdio.h>    // sscanf
#include <stdlib.h>   // exit, atoi
#include <string.h>	  // strchr, strcasecmp
#include "vector.cpp" // my vector container class
#include "globals.h"  // struct period_ent, clear, debug_print

void mk_period_ent( period_ent &, char*, char*, char*, char*, char*, char* );

void init(const char* inifile, vector<period_ent> & timelist, int & count)
{
	// Start reading in from our file

	ifstream in;
	char buf[80] = { 0 };
	in.open(inifile);
	if ( ! in.good() )
	{
		cerr << "unable to open " << inifile << endl;
		exit(1);
	}

	char minbuf[80] = { 0 };
	char hrbuf[80] = { 0 };
	char dmonbuf[80] = { 0 };
	char monbuf[80] = { 0 };
	char dwkbuf[80] = { 0 };
	char labelbuf[80] = { 0 };
	period_ent current;

	timelist.resize(10);
	count = 0;

	cout << endl;

	while ( in.getline(buf,80) )
	{
		if ( buf[0] == '#' || buf[0] == '\0' )
			continue;
		sscanf( buf, " %s %s %s %s %s %s ", minbuf,hrbuf,dmonbuf,monbuf,dwkbuf,labelbuf );
#ifdef DEBUG2
		cout << '[' << minbuf << ']' << '[' << hrbuf << ']' << '[' << dmonbuf << ']';
		cout << '[' << monbuf << ']' << '[' << dwkbuf << ']' << '[' << labelbuf << ']';
		cout << endl;
#endif
		clear(current);
		mk_period_ent( current, minbuf,hrbuf,dmonbuf,monbuf,dwkbuf,labelbuf );
		if ( count >= timelist.length() )
			timelist.resize(count+10);
		timelist[count] = current;
		++count;
	}
	in.close();

#ifdef DEBUG2
	for ( int i=0; i<count; ++i )
		debug_print( timelist[i] );
	cin.get();
#endif

}

int month_lookup ( char* );
int weekday_lookup ( char* s );

const char* month_table[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int month_lookup ( char* s )
{
	
	// Returns month number given name
	int i;
	for ( i=0 ; i<12; ++i )
		if ( strncasecmp( s, month_table[i], 3 ) == 0 )
			return i;

	cerr << "period: error in period.ini; invalid month \"" <<s<< "\"\n";
	exit(1);

	return 0;	// just to keep the compiler warnings away
}

const char* weekday_table[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

int weekday_lookup ( char* s )
{
	
	// Returns weekday number given name
	int i;
	for ( i=0 ; i<7; ++i )
		if ( strncasecmp( s, weekday_table[i], 3 ) == 0 )
			return i;

	cerr << "period: error in period.ini; invalid weekday \"" <<s<< "\"\n";
	exit(1);

	return 0;	// just to keep the compiler warnings away
}


void mk_period_ent( period_ent & p, char* minbuf, char* hrbuf,
                    char* dmonbuf, char* monbuf, char* dwkbuf, char* labelbuf )
{
	int i;

	// okay, any of the buffers might have ranges (-) or lists (,)
	// so far, only ranges are implemented, and only for dwk and month

	// Minute
	if ( strcmp(minbuf, "*") == 0 )
		for ( i=0; i<60; ++i )
			p.min[i] = true;
	else
		p.min[ atoi(minbuf) ] = true;

	// Hour
	if ( strcmp(hrbuf, "*") == 0 )
		for ( i=0; i<24; ++i )
			p.hr[i] = true;
	else
		p.hr[ atoi(hrbuf) ] = true;
		
	// Day of the month
	if ( strcmp(dmonbuf, "*") == 0 )
		for ( i=0; i<31; ++i )
			p.monthday[i] = true;
	else
		p.monthday[ atoi(dmonbuf)-1 ] = true;

	// Month
	if ( strcmp(monbuf, "*") == 0 )
		for ( i=0; i<12; ++i )
			p.month[i] = true;
	// Check for a range (-) (but no lists (,) yet)
	else if ( strchr(monbuf, '-') )
	{
		// This one has a hyphen.
		// Save a pointer to beginning of start day string
		// (We can get away with this because month_lookup only compares
		//  the first three characters of its buffer.)
		char* start = monbuf;
		// Save a pointer to beginning of hyphen between the two
		char* stop = strchr(monbuf, '-');
		// Then move it past the hyphen to the beginning of stop day string
		++stop;
		// Now just true everthing between (inclusive)
		for ( i=month_lookup(start); i<=month_lookup(stop); ++i )
			p.month[i] = true;
	}
	else
		p.month[ month_lookup(monbuf) ] = true;

	// Day of the week
	if ( strcmp(dwkbuf, "*") == 0 )
		for ( i=0; i<7; ++i )
			p.weekday[i] = true;
	// Check for a range (-) (but no lists (,) yet)
	else if ( strchr(dwkbuf, '-') )
	{
		// This one has a hyphen.
		// Save a pointer to beginning of start day string
		// (We can get away with this because weekday_lookup only compares
		//  the first three characters of its buffer.)
		char* start = dwkbuf;
		// Save a pointer to beginning of hyphen between the two
		char* stop = strchr(dwkbuf, '-');
		// Then move it past the hyphen to the beginning of stop day string
		++stop;
		// Now just true everthing between (inclusive)
		for ( i=weekday_lookup(start); i<=weekday_lookup(stop); ++i )
			p.weekday[i] = true;
	}
	else
		p.weekday[ weekday_lookup(dwkbuf) ] = true;

	// Label
	strcpy( p.label, labelbuf );

}

