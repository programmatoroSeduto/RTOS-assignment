
#include "timespectools.h"

long int tspec_toNumber( struct timespec tspec )
{
    return tspec.tv_sec * 1000000000 + tspec.tv_nsec;
}

struct timespec tspec_plus( struct timespec tspec, int delta )
{
    tspec.tv_nsec += delta;
    if( tspec.tv_nsec >= 1000000000 )
    {
        tspec.tv_sec++;
        tspec.tv_nsec %= 1000000000;
    }

    return tspec;
}

void tspec_plus_p( struct timespec* tspec, int delta )
{
    tspec->tv_nsec += delta;
    if( tspec->tv_nsec >= 1000000000 )
    {
        tspec->tv_sec++;
        tspec->tv_nsec %= 1000000000;
    }
}

long int tspec_diff( struct timespec t1, struct timespec t2 )
{
    return tspec_toNumber(t2) - tspec_toNumber(t1);
}