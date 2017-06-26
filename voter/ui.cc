#include <string.h>
#include <curses.h>
#if 0
#include <unistd.h>
#include <stdio.h>
#include <fstream.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#include "globals.h"

static void draw_line(int,attr_t=A_NORMAL);

const int HIGHLIGHT = A_REVERSE;
char* spaces = NULL;

void ui_init()
{
	initscr();	/* curses init     */
	cbreak();					/* */
	noecho();					/* */
	scrollok(stdscr, true);		/* */
	keypad(stdscr, true);		/* */

	// set up array of spaces (don't ask)
	spaces = new char[COLS];
	for ( int n=0; n<COLS-1; ++n )
		spaces[n] = ' ';
	spaces[COLS-1] = '\0';
}


void do_ui()
{
	int i;
	int n = ( num_ballot_items < LINES-1 ) ? num_ballot_items : LINES-1;
	int top_pos = 0;
	int bottom_pos = LINES - 2;
	int active_ballot = 0;
	int cur_line = 0;

	move(0,0);
	for ( i=0; i<n; ++i ) {
		if ( i == active_ballot )
			draw_line(i,HIGHLIGHT);
		else
			draw_line(i);
	}	

	int ch;

	ch = getch();
	while ( ch != 'q' ) {
		switch (ch)
		{
			case 'k':	
			case KEY_UP:
				if ( active_ballot == 0 )
					break;
				// check if we're going off top
				if ( cur_line == top_pos ) {
					// put current line back in NORMAL text
					move(cur_line,0);
					draw_line(active_ballot);
					// go ahead and scroll
					scrl(-1);
					move(cur_line,0);	// just in case we moved
					// and now write the new line, which is now active
					--active_ballot;
					draw_line(active_ballot,HIGHLIGHT);
					// ...that should do it.
				}	
				else {
					// don't worry about scrolling, just draw
					move(cur_line-1,0);
					draw_line(active_ballot-1,HIGHLIGHT);
					draw_line(active_ballot);
					--active_ballot;
					--cur_line;
				}	
				refresh();
				break;
			case 'j':	
			case KEY_DOWN:
				if ( active_ballot == num_ballot_items-1 )
					break;
				// first check if we're running off bottom
				if ( cur_line == bottom_pos ) {
					// put current line back in NORMAL text
					move(cur_line,0);
					draw_line(active_ballot);
					// go ahead and scroll
					scrl(1);
					move(cur_line,0);	// just in case we moved
					// and now write the new line, which is now active
					++active_ballot;
					draw_line(active_ballot,HIGHLIGHT);
					// ...that should do it.
				}	
				else {
					// don't worry about scrolling, just draw
					move(cur_line,0);
					draw_line(active_ballot);
					++active_ballot;
					draw_line(active_ballot,HIGHLIGHT);
					++cur_line;
				}	
				refresh();
				break;
			case '\n':
			case ' ':
			case 'Y':
			case 'y':
				if ( votes[active_ballot] > 0 )
					votes[active_ballot] = 0;
				else
					votes[active_ballot] = 25;
				move(cur_line,0);
				draw_line(active_ballot,HIGHLIGHT);
				break;	
			case 'x':
			case 'X':
			case 'n':
			case 'N':
				if ( votes[active_ballot] < 0 )
					votes[active_ballot] = 0;
				else
					votes[active_ballot] = -25;
				move(cur_line,0);
				draw_line(active_ballot,HIGHLIGHT);
				break;	
			case KEY_HOME: 
				refresh();
				break;
			case KEY_END:
				refresh();
				break;
			case KEY_PPAGE:
				refresh();
				break;
			case KEY_NPAGE:
				refresh();
				break;
		}
		ch = getch();
	}
	move(bottom_pos, 0);
	printw("\n");
	refresh();

	return;
}


void draw_line(int i, attr_t attr )
{
	char c = ' ';
	attr_t saved_attr;
	short junk;
	int n = 0;

	attr_get( &saved_attr, &junk, NULL);

	if ( votes[i] < 0 )
		c = 'N';
	else if ( votes[i] > 0 )
		c = 'Y';

	n = strlen(ballot_names[i]) + 4;
	if ( attr == saved_attr )
		printw("[%c] %s%s\n",c,ballot_names[i],spaces+n);
	else {
		attrset(attr);
		printw("[%c] %s%s\n",c,ballot_names[i],spaces+n);
		attrset(saved_attr);
	}
}	

