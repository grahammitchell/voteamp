#ifndef __GLOBALS_H
#define __GLOBALS_H

struct period_ent
{
	bool min[60];
	bool hr[24];
	bool monthday[31];
	bool month[12];
	bool weekday[7];
	char label[80];
};

void debug_print( const period_ent & );
void clear( period_ent & );

#endif
