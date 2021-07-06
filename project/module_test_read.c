#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

/*
	HOW TO MOUNT THE MODULE
cd module
sudo ./mount.sh
sudo ./mount.sh && dmesg
cd ..

	TEST - WRITE
sudo ./module_test_write [2 1] [1 [2 [3 2] 3] 1]
dmesg

	TEST - READ
sudo ./module_test_read 
sudo ./module_test_read && dmesg

	HOW TO UNMOUNT THE MODULE
cd module
sudo ./unmount.sh && dmesg
cd ..

	LAZY RUNNING - TEST WRITE
sudo ./restart_module.sh && sudo ./module_test_write [2 1] [1 [2 [3 2] 3] 1]
sudo ./restart_module.sh && sudo ./module_test_write [2 1] [1 [2 [3 2] 3] 1] && dmesg

	LAZY RUNNING - TEST READ
sudo ./restart_module.sh && sudo ./module_test_read
sudo ./restart_module.sh && sudo ./module_test_read && dmesg
*/

#define DEV_FILE "/dev/RTOS_module_GF_c"
#define FATAL_ERROR( msg ) \
	{ \
	printf( "(errno=%d) ", errno ); \
	fflush( stdout ); \
	perror( msg ); \
	exit( EXIT_FAILURE ); \
	} 

// write what is passed from the command line
//    ./module_test_read
int main( int argc, char* argv[] )
{
	// open the module
	printf( "opening special file: %s ... \t", DEV_FILE );
	int dev_fd = open( DEV_FILE, O_RDONLY );
	if ( dev_fd < 0 )
	{
		printf( "\n" );
		FATAL_ERROR( "ERROR in open()" );
	}
	printf( "SUCCESS: file opened.\n" );
	
	// read
	printf( "requiring dump... " );
	if( read( dev_fd, argv[0], sizeof( argv[0] ) ) < 0 )
	{
		FATAL_ERROR( "ERROR in read()" );
	}
	printf( "OK. (see dmesg)\n" );
	
	// close the module
	printf( "closing ...\n" );
	close( dev_fd );
	
	exit( EXIT_SUCCESS );
}
