/*
 * To test the functionality of the sleep.ko module use this file.
 *
 * Firstly we need to make the /proc/sleep file busy by applying the
 * following command:
 * 	'tail -f /proc/sleep &'
 *
 * It will keep /proc/sleep open in background. In this time we will
 * run:
 * 	'./non_block /proc/sleep'
 *
 * The process should signal out that the file is busy and we cannot
 * access it at the moment.
 *
 * To close the first process simply run:
 * 	'kill %1'
 *
 * Then when we retry to open the file with non_block, it will succeed.
 *
 * To actually write some data in /proc/sleep, simply run:
 *	'echo "something" > /proc/sleep
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_BYTES 2048

int main(int argc, char *argv[]) {

	int fd;
	size_t read_bytes;
	char buffer[MAX_BYTES];

	if (argc != 2) {

		printf("Usage: %s <filename>\n", argv[0]);
		puts("Reads content from a file but if the file is busy don't wait for its content");
		exit(EXIT_FAILURE);

	}

	// Open the file and handle the errors that may occur
	fd = open(argv[1], O_RDONLY | O_NONBLOCK);

	if (fd == -1) {

		if (errno == EAGAIN) {
			puts("File is busy");
		} else {
			puts("Open failed");
		}

		exit(EXIT_FAILURE);

	}

	do {
		int i;

		// Read the content and handle the errors that may occur
		read_bytes = read(fd, buffer, MAX_BYTES);

		if (read_bytes == -1) {

			if (errno == EAGAIN) {
				puts("File is busy");
			} else {
				puts("Read error");
			}

			exit(EXIT_FAILURE);
		}

		if (read_bytes > 0) {
			for (i = 0; i < read_bytes; i++) {
				putchar(buffer[i]);
			}
		}

	} while (read_bytes > 0);

	return EXIT_SUCCESS;

}
