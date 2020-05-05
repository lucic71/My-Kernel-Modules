/*
 * This module uses command line arguments.
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>

#define AUTHOR      "Lucian"
#define DESCRIPTION "A simple Hello World module"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

// uninitialized global variables are 0
static short int short_int = 1;

module_param(short_int, short, S_IRUSR);
MODULE_PARM_DESC(short_int, " This is a short integer given by the user");

static int __init hello_init(void) {
	
	printk(KERN_INFO "Hello, World with command line arguments\n");
	printk(KERN_INFO "short_int is: %d\n", short_int);

	return 0;

}

static void __exit hello_exit(void) {
	
	printk(KERN_INFO "Goodbye with command line arguments");

}

module_init(hello_init);
module_exit(hello_exit);
