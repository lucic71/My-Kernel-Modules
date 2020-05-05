/*
 * start.c and stop.c are compiled together and result a single module
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>

void my_cleanup(void) {

	printk(KERN_INFO "Goodbye from stop source");

}
