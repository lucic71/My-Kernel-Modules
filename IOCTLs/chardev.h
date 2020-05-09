#ifndef CHARDEV_H_
#define CHARDEV_H_

/*
 * This file will be shared between the userspace process that wants to
 * use ioctl's to control the kernel module and the kernel module.
 *
 */

/*
 * Name of the device file.
 *
 */
#define DEVICE_NAME "char_device"

/*
 * Major number of the device. We cannot rely on dynamic registration
 * anymore because ioctls need to know it.
 *
 * It is used in IOCTL_SET_MSG.
 *
 */
#define CHRDEV_MAJOR 100

/*
 * Set the message of the device driver. _IOR means that we are reading
 * data from the device driver. The driver will be allowed to return
 * sizeof(data_type) bytes to user. To see more details visit:
 * https://stackoverflow.com/questions/22496123/what-is-the-meaning-of-this-macro-iormy-macig-0-int
 *
 * Arguments
 * ---------
 *
 * 1. type      - major device number
 * 2. number    - number of the command, needs to be different to distinguish
 * 	between ioctls
 * 3. data_type - type of the data going into the kernel or coming out of the 
 * 	kernel
 *
 */
#define IOCTL_SET_MSG _IOR(CHRDEV_MAJOR, 0, char *)

/*
 * Get the message of the device driver. This IOCTL is used for output, to get
 * the message of the device driver.
 *
 */
#define IOCTL_GET_MSG _IOR(CHRDEV_MAJOR, 1, char *)

/*
 * Get the n-th byte of the message. This IOCTL is used for both input and
 * output, It receives from the user a number, n, and returns Message[n].
 *
 */
#define IOCTL_GET_NTH_BYTE _IOWR(CHRDEV_MAJOR, 2, int)

#endif
