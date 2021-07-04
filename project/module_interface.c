#include "module_interface.h"

int __module_file_descriptor = -1;
int __module_no_send_mode = 0;

int module_init()
{
    pthread_mutexattr_init( &mx_attr );
    pthread_mutexattr_setprotocol( &mx_attr, PTHREAD_PRIO_INHERIT );
    pthread_mutex_init( &mx, &mx_attr );

    return module_open();
}

int module_open()
{
    __module_file_descriptor = open( MODULE_PATH, MODULE_PERMS );
    return __module_file_descriptor;
}

int module_close()
{
    return close( __module_file_descriptor );
}

int module_start_string( int id )
{
    if( !__module_no_send_mode ) 
    {
        pthread_mutex_lock( &mx );
        printf( "[%d ", id );
        pthread_mutex_unlock( &mx );
    }
    return 0;
}

int module_stop_string( int id )
{
    if( !__module_no_send_mode )
    {
        pthread_mutex_lock( &mx );
        printf( "%d] ", id );
        pthread_mutex_unlock( &mx );
    }
    return 0;
}

int module_switch_no_send()
{
    __module_no_send_mode = !__module_no_send_mode;
    return 0;
}
