#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "chardev.h"

#define SUCCESS 0
#define BUFLEN  100

/*
 * Variable used to prevent concurent acces into the same device.
 *
 */
static int Device_Open = 0;

/*
 * The message the device will give when asked.
 *
 */
static char Message[BUFLEN];

/*
 * How far did the process reading the message get? It is useful if
 * Message is larger than the buffer we get to fill in device_read.
 *
 */
static char *Message_Pointer;

/*
 * This is called whenever a process attempts to open the device file.
 *
 */
static int device_open(struct inode *inode, struct file *file) {

	return SUCCESS;

}

/*
 * This structure will hold the functions to be called when a process
 * does something to the device we created. Since a pointer to this
 * structure is kept in the devices table, it cannot be local to
 * this module. It is automatically filled with NULL for unimplemented
 * functionalities.
 *
 * To see the signature of the functions needed to fill the fields in
 * this structure, please see:
 * https://www.tldp.org/LDP/lkmpg/2.4/html/c577.htm
 *
 */
struct file_operations Fops = {
	.open = device_open
};

int init_module() {

	int ret_val;

	/*
	 * We no longer use 0 as the first argument as we did in the last 
	 * lessons because now we want to tell the kernel to register the
	 * device with the major number CHRDEV_MAJOR.
	 *
	 */
	ret_val = register_chrdev(CHRDEV_MAJOR, DEVICE_NAME, &Fops);

	if (ret_val < 0) {
		printk(KERN_ALERT "Error: register_chdrev: %d\n", ret_val);
		return ret_val;
	}

	printk(KERN_INFO "Successfully registered device with the major number %d\n",
		CHRDEV_MAJOR);

	printk(KERN_INFO "If you want to talk to the device driver you have to"
		" create a device file. Use the following command: \n"
		"`mknod %s c %d 0`\n", DEVICE_NAME, CHRDEV_MAJOR);

	return SUCCESS;

}

void cleanup_module() {

	/*
	 * Unregister the device. Keep in mind that since 2.6.12 unregister_chrdev
	 * does not return a value anymore becuase it was reported that it always
	 * returned 0 making it meaningless.
	 *
	 * See:
	 * https://stackoverflow.com/questions/3237384/how-to-find-if-unregister-chrdev-call-was-successful
	 *
	 */
	unregister_chrdev(CHRDEV_MAJOR, DEVICE_NAME);
}
