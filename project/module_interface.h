#ifndef __MODULE_INTERFACE_H__
#define __MODULE_INTERFACE_H__

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"

#include <pthread.h>

#define MODULE_PATH "/dev/random"
#define MODULE_PERMS O_WRONLY

// channel with the module
extern int __module_file_descriptor;
// no-send mode
extern int __module_no_send_mode;
// mutex
pthread_mutex_t mx;
pthread_mutexattr_t mx_attr;

// initialization steps for the module (opening the channel, initializing mutex with priority inheritance)
int module_init();

// open the channel with the module
int module_open();

// close the clannel
int module_close();

// send a string like [3
int module_start_string( int id );

// send a string like 3]
int module_stop_string( int id );

// switch no_send mode
int module_switch_no_send();

#endif