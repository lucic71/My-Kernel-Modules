/*
 * start.c and stop.c are compiled together and result a single module
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>

int my_init(void) {

	printk(KERN_INFO "Hello world from start source\n");
	return 0;

}
