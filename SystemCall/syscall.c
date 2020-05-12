#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>	// The list of system calls

#include <linux/kallsyms.h>
#include <linux/highmem.h>
#include <asm/unistd.h>

/*
 * This license must be used because we want to use the kallsyms_lookup_name 
 * function.
 *
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LUCIAN");
MODULE_DESCRIPTION("A simple module that changes an entery in sys_call_table");

#define SUCCESS 0

/*
 * sys_call_table is no longer exported since 2.6.x. So the following
 * definition of sys_call_table won't be valid:
 *
 * 	extern void **sys_call_table;
 *
 * The hacky way to access the sys_call_table now is to get its address
 * using its symbol name using the kallsyms_lookup_table. Or it can be
 * hardcoded by taking its address from /boot/System.map-3.13.0-32-generic.
 * (3.10.0 is my kernel version)
 *
 */
static void **sys_call_table;

/*
 * This is the original system call from sys_call_table. We will restore the
 * function when removing the module because I want to keep the sys_call_table
 * intact.
 *
 */
asmlinkage int (*original_call) (const char *, int, int);

/*
 * This is the modified system call. It is just a POC, but it can do other cool
 * things.
 *
 */
asmlinkage int our_sys_open(const char *filename, int flags, int mode) {
	
	printk(KERN_INFO "Open system call hijacked\n");
	return original_call(filename, flags, mode);

}

/*
 * These functions are setting the page access permissions. We need them because
 * the sys_call_table is by default RO. If we want to write a pointer inside it
 * we need to change the permission of the page in which the sys_call_table
 * is located to RW.
 *
 */
static int make_page_RW(unsigned long _address) {

	unsigned int level;

	pte_t *pte = lookup_address(_address, &level);

	if (pte->pte & ~_PAGE_RW) {
		pte->pte |= _PAGE_RW;
	}

	return 0;

}

static int make_page_RO(unsigned long _address) {

	unsigned int level;

	pte_t *pte = lookup_address(_address, &level);

	pte->pte &= ~_PAGE_RW;

	return 0;
}


/*
 * Entry point in the module. It reads an entry from sys_call_table and puts it in
 * original_call, after that it overwrites that entry with our systemcall.
 *
 */
static int __init start(void) {

	sys_call_table = (void *) kallsyms_lookup_name("sys_call_table");
	original_call = sys_call_table[__NR_open];

	make_page_RW((unsigned long) sys_call_table);
	sys_call_table[__NR_open] = our_sys_open;
	make_page_RO((unsigned long) sys_call_table);

	return SUCCESS;

}

/*
 * Exit point from the module. Put back the original_call.
 *
 */
static void __exit exit(void) {

	make_page_RW((unsigned long) sys_call_table);
	sys_call_table[__NR_open] = original_call;
	make_page_RO((unsigned long) sys_call_table);

}

module_init(start);
module_exit(exit);
