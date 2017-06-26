#include <iostream.h>
#include "globals.h"

void debug_print( const period_ent & p )
{
	int i;
	cout << p.label << endl;
	cout << "Sun\tMon\tTue\tWed\tThu\tFri\tSat\n";
	for ( i=0; i<7; ++i )
		cout << ( p.weekday[i] ? '*' : ' ' ) << '\t';
	cout << endl;
	cout << "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec\n ";
	for ( i=0; i<12; ++i )
		cout << ( p.month[i] ? '*' : ' ' ) << "   ";
	cout << endl;
	cout << "          1         2         3\n";
	cout << "0123456789012345678901234567890\n";
	for ( i=0; i<31; ++i )
		cout << ( p.monthday[i] ? '*' : ' ' );
	cout << endl;
	cout << "          1         2\n";
	cout << "012345678901234567890123\n";
	for ( i=0; i<24; ++i )
		cout << ( p.hr[i] ? '*' : ' ' );
	cout << endl;
	cout << "          1         2         3         4         5\n";
	cout << "012345678901234567890123456789012345678901234567890123456789\n";
	for ( i=0; i<60; ++i )
		cout << ( p.min[i] ? '*' : ' ' );
	cout << endl;
}
	

void clear( period_ent & p )
{
	int i;
	for ( i=0; i<60; ++i )
		p.min[i] = false;
	for ( i=0; i<24; ++i )
		p.hr[i] = false;
	for ( i=0; i<31; ++i )
		p.monthday[i] = false;
	for ( i=0; i<12; ++i )
		p.month[i] = false;
	for ( i=0; i<7; ++i )
		p.weekday[i] = false;
	for ( i=0; i<80; ++i )
		p.label[i] = '\0';
}
