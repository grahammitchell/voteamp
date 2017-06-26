// voteserver 0.1.0 (c) 1999 Graham Mitchell - 24 Jul 99

// globals.h
// Global variables shared among all files.

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

#ifndef __GLOBALS_H

extern int default_port;
extern bool verbose;
extern bool detach;
extern char* allowfilename;
extern char* denyfilename;
extern char* statefilename;
extern char* resultsfilename;
extern char* ballotfilename;
extern const char* inifilename;
extern int voting_threshold;
extern int auth_mode;
extern int voting_frequency;
extern int voting_maximum;
extern int voting_duration;
extern int clamp_value;

const int MAXLINE = 512;

#define __GLOBALS_H
#endif
