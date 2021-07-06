#include "module_interface.h"

int __module_file_descriptor = -1;
int __module_no_send_mode = 0;

int module_init()
{
	// semaphore (mutex) with Priority Inheritance
    pthread_mutexattr_init( &mx_attr );
    pthread_mutexattr_setprotocol( &mx_attr, PTHREAD_PRIO_INHERIT );
    pthread_mutex_init( &mx, &mx_attr );
	
	// try and open the module
    return module_open();
}



int module_open()
{
    __module_file_descriptor = open( MODULE_PATH, MODULE_PERMS );
    return __module_file_descriptor;
}



int module_close()
{
    close( __module_file_descriptor );
    __module_file_descriptor = -1;
    return 0;
}



int module_start_string( int id )
{
	if ( __module_file_descriptor <= 0 )
	{
		// module not yet opened!
		return -1;
	}
	
	char msg_str[3];
	sprintf( msg_str, "[%d", id );
	int len = 2;
	
	int retval = 0;
	
    if( !__module_no_send_mode ) 
    {
        pthread_mutex_lock( &mx );
        if ( write( __module_file_descriptor, msg_str, len ) < 0 )
        {
        	// error in writing start string
        	retval = -1;
        }
        pthread_mutex_unlock( &mx );
    }
    
    return 0;
}



int module_stop_string( int id )
{
    if ( __module_file_descriptor <= 0 )
	{
		// module not yet opened!
		return -1;
	}
	
	char msg_str[3];
	sprintf( msg_str, "%d]", id );
	int len = 2;
	
	int retval = 0;
	
    if( !__module_no_send_mode ) 
    {
        pthread_mutex_lock( &mx );
        if ( write( __module_file_descriptor, msg_str, len ) < 0 )
        {
        	// error in writing start string
        	retval = -1;
        }
        pthread_mutex_unlock( &mx );
    }
    
    return 0;
}



int module_switch_no_send()
{
	pthread_mutex_lock( &mx );
    __module_no_send_mode = !__module_no_send_mode;
    pthread_mutex_unlock( &mx );
    
    return 0;
}



// dump command on dmesg
int module_dump( )
{
    if ( __module_file_descriptor <= 0 )
	{
		// module not yet opened!
		return -1;
	}
	
	int retval = 0;
	char* buf = "5";
	
    pthread_mutex_lock( &mx );
    if ( read( __module_file_descriptor, buf, 1 ) < 0 )
    {
    	// error in writing start string
    	retval = -1;
    }
    pthread_mutex_unlock( &mx );
    
    return retval;
}











