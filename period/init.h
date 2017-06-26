#ifndef __INIT_H
#define __INIT_H

#include "vector.cpp" // my vector container class
#include "globals.h"  // struct period_ent, clear, debug_print

void init(const char* inifile, vector<period_ent> & timelist, int & count);

#endif
