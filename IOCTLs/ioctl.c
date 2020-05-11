/*
 * This is the process that controls the kernel module.
 *
 */

/*
 * The header file of the kernel module also needs to be included here
 * because the process must know the IOCTL numbers.
 *
 */
#include "chardev.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFLEN 100

/*
 * Functions for the IOCTL calls.
 *
 */
void ioctl_get_nth_byte(int fd) {

	int i;
	char c;

	printf("ioctl_get_nth_byte message: ");

	i = 0;
	do {
		c = ioctl(fd, IOCTL_GET_NTH_BYTE, i++);
		if (c < 0) {
			printf("ioctl_get_nth_byte failed at the %dth byte\n", i);
			exit(EXIT_FAILURE);
		}

		putchar(c);

	} while (c != 0);
	
	if (i == 1) {
		printf("there is no message at the moment");
	}

	putchar('\n');

}

void ioctl_set_msg(int fd, char *message) {

	int ret_val;

	ret_val = ioctl(fd, IOCTL_SET_MSG, message);

	if (ret_val < 0) {

		printf("ioctl_set_msg failed: %d\n", ret_val);
		exit(EXIT_FAILURE);

	}

	printf("ioctl_set_msg internal buffer set to: %s\n", message);
}

void ioctl_get_msg(int fd) {

	int ret_val;
	char message[BUFLEN];

	/*
	 * This may cause overflow because the process does not know the size of
	 * the internal buffer of the module. In a real program, two IOCTLs
	 * must be used. One to tell the module how much is allowed to write
	 * and one to actually get the buffer.
	 *
	 */

	ret_val = ioctl(fd, IOCTL_GET_MSG, message);

	if (ret_val < 0) {

		printf("ioctl_get_msg failed: %d\n", ret_val);
		exit(EXIT_FAILURE);

	}

	printf("ioctl_get_msg message: %s\n", message);

}


int main() {

	int fd;
	char *msg = "Message used form IOCTLs\n";

	fd = open(DEVICE_NAME, 0);
	if (fd < 0) {
		printf("Cannot open device file: %s\n", DEVICE_NAME);
		exit(EXIT_FAILURE);
	}

	ioctl_set_msg(fd, msg);
	ioctl_get_nth_byte(fd);
	ioctl_get_msg(fd);

	close(fd);

	return EXIT_SUCCESS;

}
