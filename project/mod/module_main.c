#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>

#define BUF_LEN 2000



/////////////////////// META

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Francesco Ganci, 4143910");
MODULE_DESCRIPTION("Sample Module for the course of RTOS RobEng 2020/2021");
MODULE_ALIAS("RTOS_module_GF");



/////////////////////// STORAGE

static char* buffer[ BUF_LEN ][2];
static int buffer_idx;
static struct semaphore buffer_sem;

// argument is supposed to be a string with length 2
static int add_trace( char* trace )
{
	if( down_interruptible( &buffer_sem ) < 0 )
		return -ERESTARTSYS;
	
	buffer[ buffer_idx ] = trace[0];
	buffer[ buffer_idx + 1 ] = trace[1];
	buffer_idx += 2;
	
	up( &buffer_sem );
	
	return 0;
}

static int clear_buffer ( void )
{
	if( down_interruptible( &buffer_sem ) < 0 )
		return -ERESTARTSYS;
	{
	int i;
	for( i=0; i<BUF_LEN, i++ )
		buffer[i] ="\0\0";
	}
	buffer_idx = 0;
	
	up( &buffer_sem );
	
	return 0
}



/////////////////////// SYSCALLS

// open
int mymodule_open(struct inode *inode, struct file *filp)
{
	// ... open ...
}

// close
int mymodule_release(struct inode *inode, struct file *filp)
{
	// ... close ...
}

// read
ssize_t mymodule_read(struct file* filp, char __user *buff, size_t count, loff_t* offp)
{
	// ... read ...
}

// write
ssize_t mymodule_write(struct file* filp, char __user *buff, size_t count, loff_t* offp)
{
	// ... write ...
}

struct file_operations RTOS_module_GF_fops = {
	.owner = THIS_MODULE,
	.open = mymodule_open,
	.release = mymodule_close,
	.read = mymodule_read,
	.write = mymodule_write,
};



/////////////////////// MODULE

static dev_t id;
static struct cdev my_cdev;

static int __init myodule_init_funct( void )
{
	printk( KERN_NOTICE " RTOS_module_GF      starting... " );
	
	clear_buffer( );
	init_mutex( &buffer_sem );
	
	int errcode = 0;
	
	// ... init ...
	printk( KERN_INFO "Allocating a major number ... " );
	if( (errcode = alloc_chrdev_region( &id, 0, 1, "RTOS_module_GF" )) < 0 )
	{
		printk( KERN_ERR "Unable to take a major number!        closing ..." );
		goto init_err_no_alloc;
	}
	else
		printk( KERN_INFO "   Success! My major is %d.", MAJOR(id) );
	
	// ... register chdev ...
	cdev_init( &my_cdev, &RTOS_module_GF_fops );
	printk( KERN_INFO "Allocating cdev ... " );
	if( (errcode = cdev_add(&my_cdev, id, 1)) < 0 )
	{
		printk( KERN_ERR "Unable to register cdev!        closing ..." );
		goto init_err_cdev;
	}
	else
		printk( KERN_INFO "   Success! " );
	
	return 0;
	
	init_err_cdev:
		dealloc_chrdev_region( id, 1 );
	init_err_no_alloc:
		// ... 
		
	return errcode;
}

static void __exit mymodule_exit_funct( void )
{
	printk( KERN_INFO "deallocating cdev ... " );
		cdev_del( &my_cdev );
	printk( KERN_INFO "deallocating major ... " );
		dealloc_chrdev_region( id, 1 );
}

module_init( myodule_init_funct );
module_exit( myodule_exit_funct );