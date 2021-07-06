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

	LAZY RUNNING - TEST WRITE then READ
sudo ./restart_module.sh && sudo ./module_test_read_write [2 1] [1 [2 [3 2] 3] 1]
sudo ./restart_module.sh && sudo ./module_test_read_write [2 1] [1 [2 [3 2] 3] 1] && dmesg

	OVERALL TEST OF THE MODULE
sudo ./test.sh && dmesg
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
//    ./module_test_write [2 1] [1 [2 [3 2] 3] 1]
int main( int argc, char* argv[] )
{
	// if no args are passed to the program, close it
	if ( argc < 2 )
	{
		printf( "ERROR: no arguments!\n" );
		printf( "USAGE:\n\t./%s aa aa aa ...\nEXAMPLE:\n\t./%s [2 1] [1 [2 [3 2] 3] 1]\n", argv[0], argv[0] );
		exit( EXIT_FAILURE );
	}
	
	// open the module
	printf( "opening special file: %s ... \t", DEV_FILE );
	int dev_fd = open( DEV_FILE, O_RDWR );
	if ( dev_fd < 0 )
	{
		printf( "\n" );
		FATAL_ERROR( "ERROR in open()" );
	}
	printf( "SUCCESS: file opened.\n" );
	
	// read dump with empty buffer
	printf( "requiring dump with empty buffer... " );
	if( read( dev_fd, argv[0], sizeof( argv[0] ) ) < 0 )
	{
		close( dev_fd );
		FATAL_ERROR( "ERROR in read()" );
	}
	printf( "OK\n" );
	
	// write into buffer
	for( int i=1; i<argc; i++ )
	{
		printf( "writing argument %d-th [%s] ...", i, argv[i] );
		if ( strlen( argv[i] ) != 2 )
		{
			printf( "DISCARDED len=%ld max len supported=2 \n", strlen( argv[i] ) );
			continue;
		}
		int retval = write( dev_fd, argv[i], 2 );
		if ( retval < 0 )
		{
			close( dev_fd );
			FATAL_ERROR( "ERROR in write()" );
		}
		printf( "SUCCESS.\n" );
	}
	
	// read
	printf( "requiring dump... " );
	if( read( dev_fd, argv[0], sizeof( argv[0] ) ) < 0 )
	{
		close( dev_fd );
		FATAL_ERROR( "ERROR in read()" );
	}
	printf( "OK. (see dmesg)\n" );
	
	// clean the buffer
	printf( "cleaning buffer... " );
	if( write( dev_fd, argv[1], 1 ) < 0 )
	{
		close( dev_fd );
		FATAL_ERROR( "ERROR in write()" );
	}
	printf( "OK. (see dmesg)\n" );
	
	// read dump with empty buffer
	printf( "requiring dump with empty buffer... " );
	if( read( dev_fd, argv[0], sizeof( argv[0] ) ) < 0 )
	{
		close( dev_fd );
		FATAL_ERROR( "ERROR in read()" );
	}
	printf( "OK\n" );
	
	// close the module
	printf( "DONE. closing ...\n" );
	close( dev_fd );
	
	exit( EXIT_SUCCESS );
}
