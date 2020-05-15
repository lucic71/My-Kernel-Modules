#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>

#include <linux/wait.h> 	// WaitQueue
#include <asm/uaccess.h>	// for put_user and get_user

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucian");
MODULE_DESCRIPTION("A proc file that keeps a queue for processes"
	" trying to access it.");

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

/*
 * Still unclear how this permissions work because I set them to 0000 and
 * the module still received and gave data when asked.
 *
 */

#define PERMISSIONS    0644

/*
 * Variable that keeps track if somebody is currently accessing the file.
 *
 */

static int FileOpen;

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

static int proc_open(struct inode *inode, struct file *file) {

	printk(KERN_DEBUG "open operation for /proc/%s triggered\n", PROC_FILE_NAME);

	/*
	 * IF the file's flags include O_NONBLOCK it means that the process does
	 * not want to wait for the proc file. In this case if the proc file
	 * is already open then the operation will fail with -EAGAIN.
	 *
	 */

	if ((file->f_flags & O_NONBLOCK) && FileOpen) {
		printk(KERN_DEBUG "Process rejected because it was nonblocking\n");
		return -EAGAIN;
	}

	/*
	 * If the file is already open, wait until it isn't anymore.
	 *
	 */

	while (FileOpen) {

		int i;
		int kill_sig = 0;

		/*
		 * This function puts the current process to sleep.
		 * Execution will be resumed right after the function call, either
		 * because somebody called wake_up(&WaitQueue) or when a signal
		 * is sent to the process.
		 *
		 * Arguments
		 * ---------
		 *
		 * 1. waitqueue to wait on
		 * 2. a condition that is checked each time the waitqueue is woken up
		 *
		 */

		wait_event_interruptible(WaitQueue, !FileOpen);

		/*
		 * If we woke up by a signal we are not blocking the return -EINTR
		 * (fail the system call). This allows processes to be killed or
		 * stopped.
		 *
		 * Explaination
		 * ------------
		 *
		 * The current is a pointer to the current process. It contains a
		 * field pending of type struct sigpending which also contains an
		 * array of signals (current->pending.signal) denoted as
		 * current->pending.signal.sig[i].
		 *
		 * current also contains a sigset_t (current->blocked) which contains
		 * blocked signals.
		 *
		 * We must iterate through all signals in sig[i] and check that they
		 * are nonblocked signals that can affect our process.
		 *
		 */

		for (i = 0; i < _NSIG_WORDS && !kill_sig; i++) {

			kill_sig = current->pending.signal.sig[i] &
				~(current->blocked.sig[i]);

		}

		if (kill_sig) {
			return -EINTR;
		}

	}

	/*
	 * If we got here it means the the file is ready to be opened and FileOpen is 0.
	 * We can now open the file
	 *
	 */

	FileOpen = 1;
	printk(KERN_DEBUG "open operation for /proc/%s was successful\n", PROC_FILE_NAME);

	return 0;
}

static int proc_close(struct inode *inode, struct file *file) {

	printk(KERN_DEBUG "close operation for /proc/%s triggered\n", PROC_FILE_NAME);

	/*
	 * Set FileOpen to zero, so one of the processes in WaitQueue will be
	 * able to open the file.
	 *
	 */

	FileOpen = 0;

	/*
	 * Wake up the processes in WaitQueue. If any process is waiting to be
	 * accepted, now is the moment to get accepted.
	 *
	 */

	wake_up(&WaitQueue);

	printk(KERN_DEBUG "close operation for /proc/%s was successful\n", PROC_FILE_NAME);
	return 0;

}

static ssize_t proc_write(struct file *file, const char __user *buffer,
	size_t length, loff_t *offset) {

	int i;

	printk(KERN_DEBUG "write operation for /proc/%s triggered\n", PROC_FILE_NAME);

	/*
	 * Put buffer in Message.
	 *
	 */

	for (i = 0; i < MESSAGE_LEN - 1 && i < length; i++) {
		get_user(*(Message + i), buffer + i);
	}

	Message[i] = 0;

	/*
	 * Return the number of written bytes.
	 *
	 */

	printk(KERN_DEBUG "write operation for /proc/%s was successful\n", PROC_FILE_NAME);
	return i;
}

static ssize_t proc_read(struct file *file, char __user *buffer,
	size_t length, loff_t *offset) {


	static int finished = 0;
	int i;
	char message[MESSAGE_LEN + 30];

	printk(KERN_DEBUG "read operation for /proc/%s triggered\n", PROC_FILE_NAME);

	/*
	 * Return 0 to signify EOF, we have nothing more to say at this
	 * point.
	 *
	 */

	if (finished) {
		finished = 0;

		printk(KERN_DEBUG "There is nothing to give at the moment\n");
		return 0;
	}

	sprintf(message, "Last input: %s\n", Message);
	for (i = 0; i < length && message[i]; i++) {
		put_user(*(message + i), buffer + i);
	}

	/*
	 * Set finished so that next time the process wants data from
	 * our internal buffer it will be rejected by the first if
	 * statement.
	 *
	 */

	finished = 1;

	printk(KERN_DEBUG "read operation for /proc/%s was successful\n", PROC_FILE_NAME);
	return i;

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
	.open = proc_open,
	.release = proc_close,
	.write = proc_write,
	.read  = proc_read
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

