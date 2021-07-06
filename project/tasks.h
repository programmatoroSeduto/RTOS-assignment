
#ifndef __TASKS_H__
#define __TASKS_H__

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h> 

#include "timespectools.h"
#include "module_interface.h"

// how many periodi tasks
#define N_PERIODIC_TASKS 3
// how many iteration for each task
#define N_CYCLES 100

// periods of the periodic tasks to be scheduled
long int Jperiod[N_PERIODIC_TASKS];
// Computation time, in cycles and in nanoseconds
long int Jcycles[N_PERIODIC_TASKS];
long int Jcycles_ns[N_PERIODIC_TASKS];

// relative references for computing the time in  which the task has to be scheduled again
struct timespec Jarrival[N_PERIODIC_TASKS][N_CYCLES];
// real arrival time and real ending time
struct timespec Jarrival_real[N_PERIODIC_TASKS][N_CYCLES];
struct timespec Jend_real[N_PERIODIC_TASKS][N_CYCLES];

// real time periodic tasks are implemented as POSIX threads
pthread_t Jthread[N_PERIODIC_TASKS];
pthread_attr_t Jthread_attr[N_PERIODIC_TASKS];

// Initialize data of the tasks
void task_init();

// the tasks invoked directly from threads
void* task1_thread( void* nothing );
void* task2_thread( void* nothing );
void* task3_thread( void* nothing );

// the bodies of the tasks
void task1_body( int cycles, int id );
void task2_body( int cycles, int id );
void task3_body( int cycles, int id );

#endif 
