
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <pthread.h>
#include <sched.h>

#include "timespectools.h"

#define N_threads 3
#define N_CYCLES 5
#define N_AVG 50

// threads, already ordered
int J_T[N_threads] = { 300000, 500000, 800000 };        // periods
int J_C[N_threads] = { 500, 600, 700 };                 // computation cycles
long int J_Cn[N_threads] = { 0, 0, 0 };                 // computation time in nanoseconds
int J_id[N_threads] = { 0, 1, 2 };                      // IDs
pthread_attr_t J_attr[N_threads];                       // thread attribute structures
long int J_arrival_times[N_threads][N_CYCLES];          // arrival time (for computing missings)
pthread_t J_thread_id[N_threads];                       // thread ids

// the body of the thread (waste time)
void thread_body_function( int C )
{
    float silly_var = -1.f;
    while( C-- >= 0 )
        silly_var = 2.0 * (rand() + rand()) - ((float) rand() * rand() ) / ((float)rand());

    return;
}

// the function directly called from pthread_create()
// id (int) : the identifier of the thread, from 0 to N-1
void* thread( void* id )
{
    int my_id = *((int*) id);
    
    struct timespec arrival_time;
    clock_gettime( CLOCK_REALTIME, &arrival_time );
    J_arrival_times[ my_id ][ 0 ] = tspec_toNumber( arrival_time );
    for( int cycle=0; cycle<N_CYCLES; cycle++ )
    {
        clock_gettime( CLOCK_REALTIME, &arrival_time );
        J_arrival_times[ my_id ][ cycle ] = tspec_toNumber( arrival_time );
        arrival_time = tspec_plus( arrival_time, J_T[my_id] );
        
        thread_body_function( J_C[my_id] );
        
        clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &arrival_time, NULL );
    }

    pthread_exit( NULL );
}

int main( int argc, char** argv )
{
    // initialize data structures!
    /** \todo  */

    // check superuser priviledges
    if( getuid() != 0 )
    {
        printf( "ERROR: not a superuser.\n" );
        return -1;
    }
    struct sched_param priomax;
    struct sched_param priomin;
    priomax.sched_priority = sched_get_priority_max( SCHED_FIFO );
    priomin.sched_priority = sched_get_priority_min( SCHED_FIFO );

    // schedulability test (only necessary condition)
    sched_setparam( pthread_self(), &priomax );
    struct timespec start_time, stop_time;
    float U = 0.f;
    for( int id = 0; id < N_threads; id++ )
    {
        float Uid = 0.f;
        //printf( " === ID%d ===\n", id );
        for( int i=0; i<N_AVG; i++ )
        {
            clock_gettime( CLOCK_REALTIME, &start_time );
            thread_body_function( J_C[ id ] );
            clock_gettime( CLOCK_REALTIME, &stop_time );
            J_Cn[id] = tspec_toNumber( stop_time ) - tspec_toNumber( start_time );
            Uid += ( J_Cn[id] ) / ( (float) J_T[id] );
            //printf( "Uid=%f\n", Uid );
        }
        U += Uid / N_AVG;
        //printf( "U=%f\n", U );
    }
    sched_setparam( pthread_self(), &priomin );
    printf( "Utilization factor = %f\n", U );
    if( U > 1.f )
    {
        printf( "U=%f the set is not schedulable.\n", U );
        return 0;
    }

    // prepare threads
    for( int id = 0; id < N_threads; id++ )
    {
        pthread_attr_init( &J_attr[ id ] );
        pthread_attr_setinheritsched(&J_attr[ id ], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy( &J_attr[ id ], SCHED_FIFO );
        struct sched_param prio;
        prio.sched_priority = priomin.sched_priority + N_threads - id;
        pthread_attr_setschedparam( &J_attr[ id ], &prio );
    }

    sleep(2);

    // execute them, and then join to them
    sched_setparam( pthread_self(), &priomax );
    for( int id = 0; id < N_threads; id++ )
        pthread_create( &J_thread_id[id], &J_attr[id], thread, (void*) &J_id[id] );
    for( int id = 0; id < N_threads; id++ )
        pthread_join( J_thread_id[id], NULL );
    sched_setparam( pthread_self(), &priomin );

    // detect missings after the simulation (only delays at the moment)
    long int t0 = J_arrival_times[0][0];
    long int T = 0;
    long int tc = t0;
    long int ta = t0;
    long int C = 0;
    for( int id = 0; id < N_threads; id++ )
    {
        T = J_T[ id ];
        C = J_Cn[ id ];
        printf( "[ === ID=%d === ] C=%ld | T=%ld | t0=%ld\n", id+1, C, T, t0 );
        int delayed = 0;
        for( int i=0; i<N_CYCLES; i++ )
        {
            tc = t0 + i * T;
            ta = J_arrival_times[id][i];
            printf( "tc=%ld | ta=%ld | ta+C=%ld | tc+T=%ld\n", tc-t0, ta-t0, ta+C-t0, tc+T-t0 );
            if( ta + C > tc + T )
            {
                delayed = 1;
                printf( "J%d delayed at cycle %d\n", id+1, i );
                break;
            }
        }
        if( !delayed ) printf( "J%d is on time.\n", id+1 );
    }

    return 0;
}