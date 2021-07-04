#ifndef __TSPEC__TOOLS__
#define __TSPEC__TOOLS__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

// conversion of a timespec into nanoseconds
long int tspec_toNumber( struct timespec tspec );

// sum tot nanoseconds to a timespec data structure
struct timespec tspec_plus( struct timespec tspec, int delta );

// sum tot nanoseconds to a timespec data structure; pointers version
void tspec_plus_p( struct timespec* tspec, int delta );

// get the difference between two timespec data structures
long int tspec_diff( struct timespec t1, struct timespec t2 );

#endif