/*
 * My first kernel module.
 */

#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void) {

	printk(KERN_INFO "Hello, World!\n");

	// if init_module returns a negative value the OS wont be able
	// to insert the module into the kernel

	return 0;
}

void cleanup_module(void) {

	printk(KERN_INFO "Goodbye!\n");

}

