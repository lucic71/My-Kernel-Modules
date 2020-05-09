#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h> 	// for put_user

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
 * Functionality
 * -------------
 *
 * In this function we need to make the device busy because we do not
 * want two processes to talk to our device at the same time. Moreover
 * we need to put our Message_Pointer at the beginnining of the message.
 *
 */

static int device_open(struct inode *inode, struct file *file) {

	printk(KERN_INFO "device_open(%p)\n", file);

	/*
	 * Make the device busy until the operation is finished.
	 *
	 */

	if (Device_Open) {
		return -EBUSY;
	}

	Device_Open = 1;

	/*
	 * Intialize the message.
	 *
	 */

	Message_Pointer = Message;

	return SUCCESS;
}

/*
 * This is called whenever a process closes the device file.
 *
 */

static int device_release(struct inode *inode, struct file *file) {

	/*
	 * Make the device available.
	 *
	 */

	Device_Open = 1;

	return SUCCESS;

}

/*
 * This is called whenever a proess which has already opened the device
 * file attempts to read from it.
 *
 * The read is from the perspective of the user.
 *
 */

static int device_read(struct file *file, char __user *buffer, size_t length,
	loff_t *offset) {

	/*
	 * Number of written bytes in the user buffer.
	 *
	 */	

	int written_bytes = 0;

	printk(KERN_INFO "device_read(%p, %p, %d)\n", file, buffer, length);

	/*
	 * If we are at the end of the message then return 0, which signifies
	 * the end of file.
	 *
	 */

	if (*Message_Pointer == 0) {
		return 0;
	}

	/*
	 * Put the data into the buffer.
	 *
	 */

	while (length && *Message_Pointer) {

		/*
		 * Because the buffer is in user space mode we need to use
		 * put_user function,
		 *
		 */

		put_user(*(Message_Pointer), buffer++);

		length--;
		written_bytes++;

	}

	printk(KERN_INFO "Read %d bytes, %d left\n", written_bytes, length);

	/*
	 * Return the number of bytes inserted in the buffer.
	 *
	 */

	return written_bytes;

}

/*
 * This function is called whenever someone tries to write to our device.
 *
 */

static ssize_t device_write(struct file *file, const char __user *buffer, 
	size_t length, loff_t *offset) {

	int i;

	printk(KERN_INFO "device_write(%p, %p, %d)\n", file, buffer, length);

	/*
	 * Take byte by byte the message from buffer.
	 *
	 */

	for (i = 0; i < length && i < BUFLEN; i++) {

		get_user(Message[i], buffer + i);

	}

	Message_Pointer = Message;

	/*
	 * Return the number of characters written in our internal buffer.
	 *
	 */

	return i;

}

/*
 * This function is called whenever a process tries to do an ioctl on our
 * device file. 
 *
 * Arguments
 * ---------
 *
 * 1. struct inode holds data about a file or a directory on disk
 * 2. struct file holds data about an open file/socket etc
 * 3. the ioctl number
 * 4. the parameter given to the ioctl function
 *
 * More info about the first two structures can be found at:
 * http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch12lev1sec6.html
 * https://www.tldp.org/LDP/tlk/ds/ds.html
 *
 */

int device_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num,
	unsigned long ioctl_param) {

	/*
	 * Description
	 * -----------
	 *
	 * i - used for interating in IOCTL_SET_MSG
	 * buf_len - keeps the length of the message coming from user space
	 * temp - char pointer to ioctl_param used for receiving the message
	 * 	from user space
	 *
	 * 	(see IOCTL_SET_MSG for more info)
	 *
	 */
	 

	int i;
	char buf_len;
	char *temp;

	/*
	 * Switch structure according to the ioctl called.
	 *
	 */

	switch (ioctl_num) {

		case IOCTL_SET_MSG:

			/*
			 * Receive a pointer to a message from user space and set
			 * that to be the device's message. Get the parameter
			 * give to ioctl by the process.
			 *
			 */

			temp = (char *) ioctl_param;

			/*
			 * Find the length of the message.
			 *
			 */

			get_user(buf_len, temp);

	}

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
	.open    = device_open,
	.release = device_release,
	.read  = device_read,
	.write = device_write
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
