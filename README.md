# RTOS Assignment - RobEng A.A. 2020/2021 

*Francesco Ganci, 4143910*

> GitHub repo of the project --- [here](https://github.com/programmatoroSeduto/RTOS-assignment) ---
> Aulaweb Specs of the project --- [AulaWeb](https://2020.aulaweb.unige.it/mod/assign/view.php?id=64169) ---

# Specifications

Here are the specs, available on [AulaWeb](https://2020.aulaweb.unige.it/mod/assign/view.php?id=64169): 

1. Design an application with 3 threads, whose periods are 300ms, 500ms, and 800ms.
2. The threads shall just "waste time", as we did in the exercise with threads.
3. Design a simple driver with only open, close, write system calls.
4. During its execution, every tasks 
	1. opens the special file associated with the driver;
	2. writes to the driver its own identifier plus open square brackets (i.e., [1, [2, or [3)
	3. close the special files
	4. performs operations (i.e., wasting time)
	5. performs (1.)(2.) and (3.) again in order to write to the driver its own identifier, but with closed square brackets (i.e., 1], 2], or 3]).
5. The write system call simply writes on the kernel log the string that was received from the thread. A typical output of the system, by reading the kernel log, may be the following \[11]\[2\[11]2]\[3\[11]3]  . This clearly shows that some threads can be preempted by other threads (if this does not happen, try to increase the computational time of longer tasks).
6. Finally, modify the code of all tasks in order to use semaphores. Every thread now protects all its operations (i) to (v) with a semaphore, which basically prevents other tasks from preempting it. Specifically, use semaphores with priority ceiling.  

# Overview and project structure

The project has the purpose to schedule three real-time tasks using *POSIX-threads* and a simple module with 4 syscalls. THe module acts as a buffer: three threads write on this buffer, according on how they're scheduled. RM policy is used for scheduling threads in user-space. 

In particular, the project has two parts:

- *the threads in user space*, implemented in `main.c` and related source files
- *the module*, implemented `RTOS_module_GF` on a kerner (tested on **linux kernel version 5.4.0-77-generic** running under **xUbuntu**)

## User-space threads

In the folder *project* you can find several `.c` source files:

- `main.c` : it contains the program for scheduling the threads. In particular, it executes threads and performs an analysis at the end of the execution. 
- `module_interface.h`, `module_interface.c` : an user interface for interacting with the module in a simple way. 
- `timespectools.h`, `timespectools.c` : tools for performing computations on `timespec` data structures.
- `tasks.h`, `tasks.c` : implementation of the tasks to be executed in real time (dummy implementation)

A **Rate Monotonic** policy is implemented using POSIX-threads. Here is a general overview on how the program works:

1. check is the program is launched as superuser: `getuid() > 0`. If the check is true, the program ends with error: the program wasn't launched as superuser. 
2. get command line arguments if any argument is provided
3. initialize the module interface: init the semaphore with policy Priority Inheritance, then open the special file
5. **schedulability test** using RM sufficient condition. 
6. set `pthread_attr` for each thread (RM is implemented)
7. **pthread create**
8. **execution analysis**, computation of the misses. 

### Tasks Stats

Refer to the file `tasks.h`. Here are some stats you can use for implementing your analysis methods if you want. 

- `Jperiod[task]` : the period of each task, in nanoseconds (see `task_init`)
- `Jcycles[task]` : the cycle number of each task Ji
- `Jcycles_ns[task]` : the estimated computation time of each task, in nanoseconds
- `Jarrival[task][no.]` : the absolute instant in which the task should start the computation in tha iteration. It is used to wake up the task
- `Jarrival_real[task][no.]` : the real instant in when the task is waken up. 
- `Jend_real[task][no.]` : the time instant in which the task ends the computation and starts waiting for the next arrival time. 

Here is how a task works. Let's consider the task Ji with period Ti and cicle number Ci; task is executed N times (N is the number of *outer cycles*, outer cycle k out of N, whereas Ci is the number of *inner cycles*). 

1. when the task starts the outer cycle, it sets the actual time in `Jarrival_real[i][k]`
2. computation is performed using `task_body()`
3. when the task has done its work, it sets the actual time in `Jend_real[i][k]`
4. the next arrival time is computed: 
    > `Jarrival[i][k] = Jarrival_real[i][k] + Jperiod[i]`
5. then, the task sleeps until the next arrival time. 

### Dummy implementation of the Tasks

The program schedules 3 threads using POSIX-threads. Each thread has a dummy implementation. Ti, aka `J[i][0]` in *main.c*, is the period lenght in milliseconds, whereas the number Ci, aka `J[i][1]` in *main.c*, is the computation time in inner cycles (there Ci is called *cycles*).

Here is the implementation of each thread, which you can find in `tasks.c`:

```c++
// *tasks.c*, lines 103, 119, 134
void task_body( int cycles, int id )
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
```

Each task has a wrapper which measures the execution time. Note that each wrapper executes the minimal set of actions, in order to waste as less time as possible. 

```c++
// *tasks.c*, lines 33, 57, 80
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
```

### Prioritisation

Priority is assigned in this way.

If **i** is the id of the task, from 0 to N-1 on a set of **N tasks**, and **P** is the minimum priority the program can have, the priority of the task is **P + 2N -2i**. This method of assigning the priorities decreases the miss rate. Check on your own modifying line 236 in **main.c* with *P + N - i*: with this method the miss rate is low, but not zero also with short tasks. 

### Evaluation of the Utilization factor

Three tasks are scheduled using RM. The computation time of each task is given by its number of inner cycles. For finding the computation time, the task is simulated a certain number of times. The program measures how much time the task requires in order to run out, then gets an average duration on all the temptatives. 

The program can check if there is harmonic relationship betweeh the three tasks considered. If the harmonic relationship doesn't hold here, the value of `3*(2^(1/3)-1)` is used as **Ulub(RM)**; otherwise, **Ulub(RM)=1**.

```c++
// *main.c* line 192 (only the structure)

float U = get_utilization_factor(); 
float Ulub = ( check_harmonic_relationship() ? 1.f : 0.7797... );

printf( "U(%f) <= Ulub(%f) ?\t", U, Ulub );
if ( U > Ulub )
{
    // ... set not schedulable, close ...
}
```

### Miss Checking

The miss rate computation is performed at the end of the execution of the set of tasks, in order to avoid waste of time in timing computations.

A **miss** is detected when the task requires too much time in order to complete its execution. The difference between the starting time and the ending time is compared with the computation time, computed before the Utilization computation. If the execution time is greater than the computation time, a miss is detected. 

A miss rate is computed as well. 

## The Module

The module along with this project, named `RTOS_module_GF`, implements a simple static buffer which is used by user space threads. You can find all the code in the folder */project/module*. 

### How to use the module - System Calls

Here are the operations implemented in the module:

- **write an ID in the buffer** : write a string of exactly two characters in the buffer. Use `write( module_fd, string of type "aa", 2 )`
- **dump** : dump the content of the buffer in the system log. Use `read( module_fd, buf, len )` in which *buf* and *len* are unused. 
- **clean-up** : empty the buffer. Use `write( module_fd, buf, 1 )`, in which *buf* is no use. 

See the test files in the folder */project/module/test* for more details, in particular the source file `module_test_read_write.c`. 

### Mount - dismount the module

Along with the code, you can find two useful scripts for mounting and dismounting the module. 

The script `./mount.sh` installs the module and creates a specia file `/dev/RTOS_module_GF_c`. 

```bash
## from /project/module folder

# mount the module, then create the special file
sudo ./mount.sh

# dismount the module
sudo ./unmount.sh
```

The output after mounting the module: if the module runned correctly, you can see this message on the console. 

```
mounting the module...
mounting the module... done. 
RTOS_module_GF 2015232 0
-> Major Number is 240
   ->   ready!   <-
```

You can check the status of the module from the system log. For instance using `dmesg`:

```
[ 9143.895407] ===== INIT =====
[ 9143.895408]  RTOS_module_GF      starting... 
[ 9143.895894] Allocating a major number ... 
[ 9143.895895]    Success! My major is 240, minor 0
[ 9143.895896] Allocating cdev ... 
[ 9143.895896]    Success! 
[ 9224.622354] deallocating cdev ... 
[ 9224.622355] Success!
[ 9224.622355] deallocating: major 240, minor 0
[ 9224.622356] Success!
[ 9224.622356] ===== EXIT =====
```

### Module interface 

In the folder *project* you can find a user space library made for interacting with the module in the user space. Here are the main methods: 

- `module_init( )` : complete init of the module. Init of a mutex with priority inheritance, and opening of the channel with the module using `module_open( )`
- `module_close( )` : close the module
- `module_start_string( int id )` : write a string *[<id>* in the buffer
- `module_stop_string( int id )` : write a string *<id>]* in the buffer
- `module_dump()` : dump the content of the buffer into the system log

# HOW TO run the project

## Simple run

Go into the folder *project*, then run the script

```bash
sudo ./run.sh
```

The script builds the entire project, then mounts the module, runs the program with the default settings, and dismounts the module when the program has runned out. 

You can find the output of the program in the file *log.log* which is created in the folder *project*. 

## Manual run

If you want other settings, for example if you want other parameters for the three tasks, you can manually run all the components.

### Build the project

First of all, you need to compile the project if you haven't doe yet it. In the folder *project* there is a script for building all the components:

```bash
sudo ./build.sh
```

The scripts builds the main program, the module, and the test programs for the module. 

### Mount the module

Use the script in the *project* folder: 

```bash
sudo ./mount.sh
```

If the module has runned correctly, you can see this output on the console:

```
mounting the module...
mounting the module... done. 
RTOS_module_GF 2015232 0
-> Major Number is 240
   ->   ready!   <-
```

In the sys log you shoud see this as well:

```
[ 9143.895407] ===== INIT =====
[ 9143.895408]  RTOS_module_GF      starting... 
[ 9143.895894] Allocating a major number ... 
[ 9143.895895]    Success! My major is 240, minor 0
[ 9143.895896] Allocating cdev ... 
[ 9143.895896]    Success! 
```

### Run the main program

You can launch the program in the default mode in folder *project*:

```bash
# default mode
sudo ./main
```

You can also configure the parameters of the threads in this way:

```bash
# change T1
sudo ./main T1

# change T1, C1
sudo ./main T1 C1

# change T1, C1, T2
sudo ./main T1 C1 T2

# ... and so on, until ...

# change everything
sudo ./main T1 C1 T2 C2 T3 C3
```

You can see the progress of the program on the console. 

### Dismount the module

For dismounting the module, call the script in *project* folder:

```bash
sudo ./unmount.sh
```

If everything is fine, you should see this message on the sys log:

```
[ 9224.622354] deallocating cdev ... 
[ 9224.622355] Success!
[ 9224.622355] deallocating: major 240, minor 0
[ 9224.622356] Success!
[ 9224.622356] ===== EXIT =====
```

# Default settings

Here are the default settings of the program:

```c++
// === *main.c* ===

// number of cycles for computing the average utilization factor for each process
#define N_AVG_CYCLES 50

// tasks parameters
//    Ji(Ti:period, Ci:inner_iterations)
int J[3][2] = {
	{300, 280},
	{500, 1000},
	{800, 2000},
};

// === *tasks.c* ===

// how many periodi tasks
#define N_PERIODIC_TASKS 3

// outer iterations
#define N_CYCLES 100
```

Utilization factor of this set: **U = 0.20...**; it can vary depending on how much the computer is busy. No harmonic relationship for this set. 

In the folder *logs* you can find some output of the program. Miss rate is always zero, because the utilization factor is very low, and also because I have a very good computer as well. 