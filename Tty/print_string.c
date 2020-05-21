#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/tty.h>		

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucian");
MODULE_DESCRIPTION("A module that prints a string to a tty");

static void print_string(char *str) {

	struct tty_struct *tty = get_current_tty();

	/*
	 * If tty is NULL the current task has no tty we can print the
	 * string to. This situation may arrise when the task is a daemon.
	 * There is nothing we can do in this situation,
	 *
	 */

	if (tty) {

		(tty->driver->ops->write) (tty, str, strlen(str));

		/*
		 * Write a newline with carriage.
		 *
		 */

		(tty->driver->ops->write) (tty, "\x0D\x0A", 2);

	} 

}

static int __init print_string_init(void) {

	print_string("module inserted, hello world!");
	return 0;

}

static void __exit print_string_exit(void) {

	print_string("module removed, farwell world!");

}

module_init(print_string_init);
module_exit(print_string_exit);
