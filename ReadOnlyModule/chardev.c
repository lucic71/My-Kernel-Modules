#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "chardev.h"

#define SUCCESS 0
#define BUFLEN  80

#define DEVICE_NAME "chardev"

/*
 * Define variables as static so they do not interfere with other global
 * variables in the kernel.
 *
 */

static int Major;		// major number assigned to the device driver
static int Device_Open = 0;	// is the device open? used to prevent multiple access to device

static char msg_buffer[BUFLEN];	// the message the device will give when asked
static char * msg_pointer;

static struct file_operations fops = {
	.read    = device_read,
	.write   = device_write,
	.open 	 = device_open,
	.release = device_release
};

int init_module(void) {
	/*
 	 * Register the device and assign a major number to it.
 	 *
 	 */
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "Major number for this device is %d. To talk to"
			 " the driver, create a new dev file with "
			 "'mkdnod /dev/%s c %d 0'",
		Major, DEVICE_NAME, Major);


	return SUCCESS;

}

void cleanup_module(void) {
	/*
	 * Unregister the device.
	 *
	 */
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "Device unregistered successfully!\n");

}

/*
 * Actual implementation of methods.
 *
 */

/*
 * Called when a process tries to open the device.
 * ex: 'cat /dev/chardev'
 *
 */
static int device_open(struct inode * inode, struct file * file) {

	static int counter = 0;

	// if the device is already open then restrict access
	if (Device_Open) {
		return -EBUSY;
	}
	Device_Open = 1;

	// take a pointer to msg in msg_pointer
	sprintf(msg_buffer, "I told you %d times Hello World!\n", counter++);
	msg_pointer = msg_buffer;

	// increment the use count
	try_module_get(THIS_MODULE);
	
	return SUCCESS;

}

/*
 * Called when a process closes the device.
 *
 */
static int device_release(struct inode * inode, struct file * file) {

	// make the device available
	Device_Open = 0;

	/*
	 * Decrement the usage count, or else once this file is opened,
	 * kernel won't be able to get rid of this module.
	 *
	 */
	module_put(THIS_MODULE);

	return SUCCESS;

}

/*
 * Called when a proces reads data from device.
 *
 */
static ssize_t device_read(struct file * filp, char * buffer, size_t length,
	loff_t * offset) {

	/*
	 * Number of bytes written to buffer.
	 *
	 */
	int bytes_write = 0;

	/*
	 * If msg_pointer is 0 it means that we got to EOF.
	 *
	 */
	if (*msg_pointer == 0) {
		return SUCCESS;
	}

	/*
	 * Put the data into the buffer.
	 *
	 */
	while (length && *msg_pointer) {
		/*
		 * The buffer is in user data segment, not in kernel segment.
		 * put_user will copy data from kernel data segment to
		 * user data segment. This works only for simple values like
		 * int, char ..
		 *
		 */

		put_user(*(msg_pointer++), buffer++);
		
		length--;
		bytes_write++;
	}

	/*
	 * Return the number of bytes put into the buffer.
	 *
	 */
	return bytes_write;

}

/*
 * Called when a process writes data to the device.
 * ex: echo "aa" > /dev/chardev
 *
 */
static ssize_t device_write(struct file * filp, const char * buffer, 
	size_t length, loff_t * offset) {

	printk(KERN_ALERT "This operation is not supported yet\n");
	return -EINVAL;

}
