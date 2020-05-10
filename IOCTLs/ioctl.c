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

int main() {

	int fd;
	char *msg = "Message used form IOCTLs\n";

	fd = open(DEVICE_NAME, 0);
	if (fd < 0) {
		printf("Cannot open device file: %s\n", DEVICE_NAME);
		exit(EXIT_FAILURE);
	}

	ioctl_get_nth_byte(fd);

	close(fd);

	return EXIT_SUCCESS;

}
