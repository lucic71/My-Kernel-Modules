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


	/*
	 * Make the device busy until the operation is finished.
	 *
	 */

	if (Device_Open) {
		printk(KERN_INFO "device is busy\n");
		return -EBUSY;
	}

	printk(KERN_INFO "device_open(%p)\n", file);
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
	printk(KERN_INFO "device released\n");

	Device_Open = 0;

	return SUCCESS;

}

/*
 * This is called whenever a process which has already opened the device
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
 * The write is from the perspective of the user.
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
 * 4. the parameter given to the ioctl function, it's strange that it is a
 * 	unsigned long and not a void *
 *
 * More info about the first two structures can be found at:
 * http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch12lev1sec6.html
 * https://www.tldp.org/LDP/tlk/ds/ds.html
 *
 */

long device_ioctl(struct file *file, unsigned int ioctl_num,
	unsigned long ioctl_param) {

	/*
	 * Description
	 * -----------
	 *
	 * i    - used for interating in IOCTL_SET_MSG (it will contain the
	 * 	length of the message contained in *ioctl_param at the end
	 * 	of the iteration)
	 *
	 * ch   - it is used to read the ioctl_param one byte at a time
	 *
	 * temp - char pointer to ioctl_param used for receiving the message
	 * 	from user space
	 *
	 * 	(see IOCTL_SET_MSG for more info)
	 *
	 */
	 

	int i;
	char ch;
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
			 * Find the length of the message by reading bytes from
			 * temp until a 0 byte is encountered.
			 *
			 */

			get_user(ch, temp);
			for (i = 0; ch && i < BUFLEN; i++, temp++) {
				get_user(ch, temp);
			}

			/*
			 * Write the message from ioctl_param in our internal
			 * message.
			 *
			 */

			device_write(file, (char *) ioctl_param, i, 0);

			break;

		case IOCTL_GET_MSG:

			/*
			 * Give the internal message to the calling process.
			 * The parameter we will be receiving is a pointer that
			 * must be filled with bytes from our message.
			 *
			 */

			i = device_read(file, (char *) ioctl_param, BUFLEN, 0);

			/*
			 * Put a 0 at the end of the buffer.
			 *
			 */

			put_user(0, (char *) ioctl_param + i);

			break;

		case IOCTL_GET_NTH_BYTE:
			
			/*
			 * Now ioctl_param must be interpreted as an integer used
			 * to index in Message.
			 *
			 */

			/*
			 * Boundary check.
			 *
			 */

			if (ioctl_param > BUFLEN - 1 || ioctl_param < 0) {
				return -EINVAL;
			}

			return Message[ioctl_param];
			
			break;

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
 * From my understanding this signatures are deprecated since 2.4.2 because
 * the file_operations structure no longer uses a ioctl field. ioctl is
 * one of the remaining parts of the kernel which runs under 
 * Big Kernel Lock.
 *
 * In newer versions of kernel the unlocked_ioctl is used to increase
 * performance.
 *
 * See this link for more details:
 * https://unix.stackexchange.com/questions/4711/what-is-the-difference-between-ioctl-unlocked-ioctl-and-compat-ioctl
 *
 */

struct file_operations Fops = {
	.open    = device_open,
	.release = device_release,
	.read    = device_read,
	.write   = device_write,
	.unlocked_ioctl   = device_ioctl
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
