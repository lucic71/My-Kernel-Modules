#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/wait.h> 	// WaitQueue

#include <asm/uaccess.h>

/*
 * Internal message.
 *
 */

#define MESSAGE_LEN 100
static char Message[MESSAGE_LEN];

/*
 * Information about the proc file.
 *
 */

#define PROC_FILE_NAME "sleep"
#define PERMISSIONS    0644

/*
 * Variable that keep track if somebody is currently accessing the file.
 *
 */

static int Opened;

/*
 * Queue of processes who want our proc file. This is just a macro that
 * declares a wait_queue_head_t and initializes it.
 *
 */
DECLARE_WAIT_QUEUE_HEAD(WaitQueue);

/*
 * File operations.
 *
 */

static int proc_open(struct inode *inde, struct file *file) {

	/*
	 * IF the file's flags include O_NONBLOCK it means that the process does
	 * not want to wait for the proc file. In this case if the proc file
	 * is already open then the operation will fail with -EAGAIN.
	 *
	 */

	if ((file->f_flags & O_NONBLOCK) && Opened) {
		return -EAGAIN;
	}

	return 0;
}

/*
 * From my understanding this structure was made opaque from 3.10 and we
 * can no more access its members to set the proc_iops or proc_fops fields.
 *
 */

static struct proc_dir_entry *Proc_File;

/*
 * File operations structure where pointers to functions like read or write
 * for our proc file lie.
 *
 */

static const struct file_operations Proc_File_Operations = {
	.read = NULL
};

static int __init sleep_entry(void) {

	Proc_File = proc_create(PROC_FILE_NAME, PERMISSIONS, NULL, &Proc_File_Operations);

	if (!Proc_File) {

		remove_proc_entry(PROC_FILE_NAME, NULL);

		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_FILE_NAME);
		return -ENOMEM;

	}

	printk(KERN_INFO "/proc/%s created\n", PROC_FILE_NAME);
	return 0;

}

static void __exit sleep_exit(void) {

	proc_remove(Proc_File);
	printk(KERN_INFO "/proc/%s removed\n", PROC_FILE_NAME);

}

module_init(sleep_entry);
module_exit(sleep_exit);

