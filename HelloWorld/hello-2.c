/*
 * Module that sets the license, author and description.
 *
 * They can be displayed using modinfo.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define AUTHOR      "Lucian"
#define DESCRIPTION "A simple Hello World module"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

static int __init hello_init(void) {

	printk(KERN_INFO "Hello, World with init and exit\n");
	return 0;

}

static void __exit hello_exit(void) {

	printk(KERN_INFO "Goodbye with init and exit\n");

}

// this lines tell the compiler where the entry point and the exit
// point of the module are located
module_init(hello_init);
module_exit(hello_exit);
