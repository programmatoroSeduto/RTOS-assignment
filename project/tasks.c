#include "tasks.h"

// Initialize data of the tasks (Ti in milliseconds)
void task_init( long int T1, long int C1, long int T2, long int C2, long int T3, long int C3 )
{
    // periods
    Jperiod[0] = T1*1000;
    Jperiod[1] = T2*1000;
    Jperiod[2] = T3*1000;

    // C cycles
    Jcycles[0] = C1;
    Jcycles[1] = C2;
    Jcycles[2] = C3;

    // C time in nanoseconds
    Jcycles_ns[0] = 0;
    Jcycles_ns[1] = 0;
    Jcycles_ns[2] = 0;

    // initialize arrival times
    for( int id = 0; id < N_PERIODIC_TASKS; id++ )
    {
        for( int j=0; j<N_CYCLES; j++ )
        {
            struct timespec tm = {0, 0};
            tspec_plus( tm, j*Jperiod[id] );
            Jarrival[id][j];
        }
    }
}

// the tasks invoked directly from threads
void* task1_thread( void* nothing )
{
    int thread_id = 0;
    struct timespec next_arrival_time = {0, 0};
    long int period = Jperiod[thread_id];
    
    clock_gettime( CLOCK_REALTIME, &next_arrival_time );
    tspec_plus_p( &next_arrival_time, period );

    for( int cycle = 0; cycle < N_CYCLES; cycle ++ )
    {
        clock_gettime( CLOCK_REALTIME, &Jarrival_real[thread_id][cycle] );
        task1_body( Jcycles[thread_id], thread_id );
        clock_gettime( CLOCK_REALTIME, &Jend_real[thread_id][cycle] );

        clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time, NULL );
        tspec_plus_p( &next_arrival_time, period );
    }

    pthread_exit( NULL );
}


void* task2_thread( void* nothing )
{
    int thread_id = 1;
    struct timespec next_arrival_time = {0, 0};
    long int period = Jperiod[thread_id];
    
    clock_gettime( CLOCK_REALTIME, &next_arrival_time );
    tspec_plus_p( &next_arrival_time, period );

    for( int cycle = 0; cycle < N_CYCLES; cycle ++ )
    {
        clock_gettime( CLOCK_REALTIME, &Jarrival_real[thread_id][cycle] );
        task2_body( Jcycles[thread_id], thread_id );
        clock_gettime( CLOCK_REALTIME, &Jend_real[thread_id][cycle] );

        clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time, NULL );
        tspec_plus_p( &next_arrival_time, period );
    }
 
    pthread_exit( NULL );
}


void* task3_thread( void* nothing )
{
    int thread_id = 2;
    struct timespec next_arrival_time = {0, 0};
    long int period = Jperiod[thread_id];
    
    clock_gettime( CLOCK_REALTIME, &next_arrival_time );
    tspec_plus_p( &next_arrival_time, period );

    for( int cycle = 0; cycle < N_CYCLES; cycle ++ )
    {
        clock_gettime( CLOCK_REALTIME, &Jarrival_real[thread_id][cycle] );
        task3_body( Jcycles[thread_id], thread_id );
        clock_gettime( CLOCK_REALTIME, &Jend_real[thread_id][cycle] );

        clock_nanosleep( CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time, NULL );
        tspec_plus_p( &next_arrival_time, period );
    }

    pthread_exit( NULL );
}


// the bodies of the tasks
void task1_body( int cycles, int id )
{
    float silly_var = -1.f;

    //printf( "[%d ", id );
    module_start_string( id );
    while( cycles-- > 0 )
        silly_var = 2.0 * (rand() + rand()) - ((float) rand() * rand() ) / ((float)rand());
    //printf( "%d] ", id );
    module_stop_string( id );

    return;
}


void task2_body( int cycles, int id )
{
    float silly_var = -1.f;

    //printf( "[%d ", id );
    module_start_string( id );
    while( cycles-- > 0 )
        silly_var = 2.0 * (rand() + rand()) - ((float) rand() * rand() ) / ((float)rand());
    //printf( "%d] ", id );
    module_stop_string( id );
    
    return;
}


void task3_body( int cycles, int id )
{
    float silly_var = -1.f;

    //printf( "[%d ", id );
    module_start_string( id );
    while( cycles-- > 0 )
        silly_var = 2.0 * (rand() + rand()) - ((float) rand() * rand() ) / ((float)rand());
    //printf( "%d] ", id );
    module_stop_string( id );
    
    return;
}