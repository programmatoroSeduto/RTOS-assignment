#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <errno.h> 

#include <sched.h>
#include <pthread.h>
#include "tasks.h"
#include "timespectools.h"
#include "module_interface.h"



// number of cycles for computing the average utilization factor for each process
#define N_AVG_CYCLES 50

// Time to wait before starting scheduling
#define SLEEP_SECS 5

// average miss rate
double miss_rate = 0.f;

// tasks
int T1 = 300;
int C1 = 20;
int T2 = 400;
int C2 = 100;
int T3 = 580;
int C3 = 400;




// It computes the utilization factor of the involved tasks.
float get_utilization_factor(  )
{
    float U = 0.f;
    float Uid = 0.f;
    struct timespec t0, t1;

    for( int id = 0; id < N_PERIODIC_TASKS; id ++ )
    {
        printf( "ID %d , period: %ld , cycles: %ld\n", id, Jperiod[id], Jcycles[id] );
        Uid = 0;
        printf( "Testing the task %d times ...\n", N_AVG_CYCLES );
        module_switch_no_send();
        for( int cycle = 0; cycle < N_AVG_CYCLES; cycle ++ )
        {
            switch (id)
            {
            case 0:
                clock_gettime( CLOCK_REALTIME, &t0 );
                task1_body( Jcycles[ 0 ], 0 );
                clock_gettime( CLOCK_REALTIME, &t1 );
            break;
            case 1:
                clock_gettime( CLOCK_REALTIME, &t0 );
                task1_body( Jcycles[ 1 ], 1 );
                clock_gettime( CLOCK_REALTIME, &t1 );
            break;
            case 2:
                clock_gettime( CLOCK_REALTIME, &t0 );
                task1_body( Jcycles[ 2 ], 2 );
                clock_gettime( CLOCK_REALTIME, &t1 );
            break;
            }

            Uid += tspec_diff( t0, t1 ) / (float) Jperiod[ id ];
            Jcycles_ns[id] += tspec_diff( t0, t1 );
            printf( "[%d] %.4f       ", cycle, tspec_diff( t0, t1 ) / (float) Jperiod[ id ]);
        }
        module_switch_no_send();
        printf( "\n" );

        U += Uid / ((float)N_AVG_CYCLES);
        Jcycles_ns[id] /= N_AVG_CYCLES;
        printf( "Average Utilization Factor for task ID %d: %.4f , computation time: %ldns\n\n", id, Uid / ((float)N_AVG_CYCLES), Jcycles_ns[id] );
    }

    return U;
}




// for each process, notify at which cycle the process gets late
// arrival time < (k+1)T - C
// end time < (k+1)T
void compute_miss()
{
    // t0 is the first absolute arrival time of the highest priority task
    long int t0 = tspec_toNumber( Jarrival_real[0][0] );

    for( int id = 0; id < N_PERIODIC_TASKS; id ++ )
    {
        long int T = Jperiod[ id ];
        long int C = Jcycles_ns[ id ];

        printf( "===== TASK J%d =====\n", id+1 );
        printf( "\tT=%ld\t\tC=%ld\n", T, C );

        int miss = 0;

        for( int k=0; k<N_CYCLES; k++ )
        {
            long int arrival_time = tspec_toNumber( Jarrival_real[id][k] ) - t0;
            long int ending_time = tspec_toNumber( Jend_real[id][k] ) - t0;
            long int diff = ending_time - arrival_time;
            if( diff > T ) ++miss;
            printf( "J%d [%d on %d] - arrival=%ld - ending=%ld - duration=%ld - miss? %s\n", id+1, k+1, N_CYCLES, arrival_time, ending_time, diff, (diff > T ? "Yes" : "No") );
        }

        printf( "\nTotal Misses for the task [%d]: [%d] on [%d] cycles \n\n", id+1, miss, N_CYCLES );

        miss_rate += miss;
    }

    miss_rate /= (3.0 * ((float) N_CYCLES ));
    printf( "Miss rate: %f\n", miss_rate );
    return;
}







int main( int argc, char** argv )
{
    // check if the program is superuser or not
    if( getuid() > 0 )
    {
        printf( "ERROR: not a superuser! PLease launch this application as su.\n" );
        exit( EXIT_FAILURE );
    }

    // open the channel with the driver
    if( module_init() < 0)
    {
        printf( "ERROR: unable to open the module! errno=%d.\n", errno ); fflush(stdout);
        perror( "error" );
        exit( EXIT_FAILURE );
    }

    // check parameters before starting
    printf( "initializing data...\n" );
    task_init( T1, C2, T2, C2, T3, C3 );
    printf( "simulation of %d real time periodic tasks.\n", N_PERIODIC_TASKS );

    // get priorities
    int prio_max_value = sched_get_priority_max( SCHED_FIFO );
    int prio_min_value = sched_get_priority_min( SCHED_FIFO );
    struct sched_param prio_min, prio_max;
    prio_max.sched_priority = prio_max_value;
    prio_min.sched_priority = prio_min_value;

    // test schedulability
    sched_setparam( pthread_self(), &prio_max );
    printf( "COmputing utilization factor...\n\n" );
    float U = get_utilization_factor();
    sched_setparam( pthread_self(), &prio_min );
    printf( "Utilization factor: %f\n", U );
    
    float Ulub = 1.f; // harmonic relationship in this case (RM scheduling algorithm)
    if ( U > Ulub )
    {
        printf( "ERROR: not schedulable set. U(%f) > Ulub(%f)\n", U, Ulub );
        exit( EXIT_FAILURE );
    }

    printf( "OK! waiting %d seconds ...\n", SLEEP_SECS );
    sleep(SLEEP_SECS);

    // setting threads
    /*
        Each thread:
        -   is explicitly scheduled
        -   its scheduling algorithm is SCHED_FIFO
        -   its priority is assigned using the formula ( minprio + N_PERIODIC_TASKS - thread_id )
    */
    printf( "Preparing threads for scheduling ...\n" );
    {
        // all the threads executed on only one processor
        cpu_set_t cset;
        CPU_ZERO (&cset);
        CPU_SET(0, &cset);

        // set attributes
        for( int id=0; id<N_PERIODIC_TASKS; id++ )
        {
            pthread_attr_t* attr = &Jthread_attr[ id ];
            pthread_attr_init( attr );
            pthread_attr_setinheritsched( attr, PTHREAD_EXPLICIT_SCHED );
            pthread_attr_setschedpolicy( attr, SCHED_FIFO );
            struct sched_param thread_prio;
            thread_prio.sched_priority = prio_min_value + 2*N_PERIODIC_TASKS - 2*id;
            pthread_attr_setschedparam( attr, &thread_prio );
            pthread_attr_setaffinity_np(&Jthread_attr[id], sizeof(cpu_set_t), &cset);
        }
    }

    printf( "scheduling...\n\n" );
    pthread_create( &Jthread[0], &Jthread_attr[0], task1_thread, NULL );
    pthread_create( &Jthread[1], &Jthread_attr[1], task2_thread, NULL );
    pthread_create( &Jthread[2], &Jthread_attr[2], task3_thread, NULL );
    pthread_join( Jthread[0], NULL );
    pthread_join( Jthread[1], NULL );
    pthread_join( Jthread[2], NULL );
    printf( "\n\n" );

    // computing delays if any
    printf( "computing misses...\n" );
    compute_miss();
    printf( "\n\n" );

    module_close();

    printf( "DONE! closing...\n\n" );
    exit( EXIT_SUCCESS );
}