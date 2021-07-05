#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>


/*
sudo insmod dtest.ko && dmesg
sudo rmmod dtest && dmesg

LOG PRINT:
https://stackoverflow.com/questions/38822599/why-printk-doesnt-print-message-in-kernel-logdmesg
*/



#define RTOS_PROJ_BUF_LEN 200000

static char buffer[ RTOS_PROJ_BUF_LEN ];
static int buffer_idx;
static struct semaphore buffer_sem;
static struct semaphore syscalls_sem;
static dev_t id;
static struct cdev my_cdev;
struct file_operations RTOS_module_GF_fops;



/////////////////////// META

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Francesco Ganci, 4143910");
MODULE_DESCRIPTION("Sample Module for the course of RTOS RobEng 2020/2021");
MODULE_ALIAS("RTOS_module_GF");



/////////////////////// STORAGE

// argument is supposed to be a string with length 2
static int add_trace( char* trace )
{
	if( down_interruptible( &buffer_sem ) < 0 )
		return -ERESTARTSYS;
	
	if( (buffer_idx + 2) >= RTOS_PROJ_BUF_LEN )
	{
		printk( KERN_ERR "ERROR: full buffer.\n" );
		return -ENOMEM;
	}
	
	buffer[ buffer_idx ] = trace[0];
	buffer[ buffer_idx + 1 ] = trace[1];
	buffer_idx += 2;
	
	up( &buffer_sem );
	
	return 0;
}

static int clear_buffer( void )
{
	int i = 0;
	
	if( down_interruptible( &buffer_sem ) < 0 )
		return -ERESTARTSYS;
	
	for( i=0; i<RTOS_PROJ_BUF_LEN; i += 2 )
	{
		buffer[i] ='\0';
		buffer[i+1] ='\0';
	}
	
	buffer_idx = 0;
	
	up( &buffer_sem );
	
	return 0;
}



/////////////////////// SYSCALLS

// open
int mymodule_open(struct inode *inode, struct file *filp)
{
	return 0;
}

// close
int mymodule_release(struct inode *inode, struct file *filp)
{
	return 0;
}

// read
ssize_t mymodule_read(struct file* filp, char __user *buff, size_t count, loff_t* offp)
{
	ssize_t retval = 0;

	// take the semaphore
	if (down_interruptible( &syscalls_sem ))
		return -ERESTARTSYS;

	// print the buffer
	printk( KERN_INFO "buffer content: %s\n", buffer );

	// free the semaphore
        up( &syscalls_sem );

	return retval;
}

// write
ssize_t mymodule_write(struct file* filp, const char __user *buff, size_t count, loff_t* offp)
{
	ssize_t retval = 0;
	char* buffer = "\0\0";
	
	printk( KERN_INFO " WRITE: count=%ld \n", count);

	// take the semaphore
	if ( down_interruptible( &syscalls_sem ) )
		return -ERESTARTSYS;

        // if count is bigger that memsize, exit
	if( count >= (RTOS_PROJ_BUF_LEN - buffer_idx) )
	{
		// error: attempting to write too many data
		printk( KERN_ERR " WRITE: ERROR not memy enough, required %ld on %d free.\n", count,(RTOS_PROJ_BUF_LEN - buffer_idx) );
		retval = -ENOMEM;
		goto out;
	}
	
	// if count is not 2, not valid
	if( count != 2 )
	{
		// error: not a valid string
		printk( KERN_ERR " WRITE: ERROR size of the string has not len 2 (count=%ld)\n", count );
		retval = -EINVAL;
		goto out;
	}
	
	// copy from user space
	printk( KERN_INFO " WRITE: copy_from_user=%ld\n", retval = copy_from_user (buffer, buff, count));
	if( retval < 0 )
	{
		// error: unable to read from use space
		printk( KERN_ERR " WRITE: ERROR unable to read from use space\n" );
		retval = -ERESTARTSYS;
		goto out;
	}

	// add the trace to the queue
	add_trace( buffer );
	
	// update retval
	retval = count;

	// free the semaphore
	out:
        up( &syscalls_sem );
	
	return retval;
}

struct file_operations RTOS_module_GF_fops = {
	.owner = THIS_MODULE,
	.open = mymodule_open,
	.release = mymodule_release,
	.read = mymodule_read,
	.write = mymodule_write,
};



/////////////////////// MODULE

static int __init mymodule_init_funct( void )
{
	int errcode = 0;
	
	printk( KERN_INFO "===== INIT =====\n" );
	printk( KERN_INFO " RTOS_module_GF      starting... \n" );
	
	sema_init( &buffer_sem, 1 );
	sema_init( &syscalls_sem, 1 );
	clear_buffer( );
	
	// major number allocation
	printk( KERN_INFO "Allocating a major number ... \n" );
	if( (errcode = alloc_chrdev_region( &id, 0, 1, "RTOS_module_GF" )) < 0 )
	{
		printk( KERN_ERR "Unable to take a major number!        closing ...\n" );
		goto init_err_no_alloc;
	}
	printk( KERN_INFO "   Success! My major is %d, minor %d\n", MAJOR(id), MINOR(id) );
	
	// register syscalls 
	cdev_init( &my_cdev, &RTOS_module_GF_fops );
	printk( KERN_INFO "Allocating cdev ... \n" );
	if( (errcode = cdev_add(&my_cdev, id, 1)) < 0 )
	{
		printk( KERN_ERR "Unable to register cdev!        closing ...\n" );
		goto init_err_cdev;
	}
	printk( KERN_INFO "   Success! \n" );
	
	
	return 0;
	
	
	init_err_cdev:
		unregister_chrdev_region( id, 1 );
	init_err_no_alloc:
		// ...
	
	return errcode;
}

static void __exit mymodule_exit_funct( void )
{
	printk( KERN_INFO "deallocating cdev ... " );
	cdev_del( &my_cdev );
	printk( KERN_INFO "Success!\n" );
	
	printk( KERN_INFO "deallocating: major %d, minor %d\n", MAJOR(id), MINOR(id) );
	unregister_chrdev_region( id, 1 );
	printk( KERN_INFO "Success!\n" );
	
	printk( KERN_INFO "===== EXIT =====\n" );
}

module_init( mymodule_init_funct );
module_exit( mymodule_exit_funct );












