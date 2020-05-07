#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROCFS_NAME "helloworld"
#define PERMS 0644

#define BUFLEN 50

/*
 * Informations about the module. Can be retrieved using `modinfo` command.
 *
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LUCIAN");
MODULE_DESCRIPTION("A simple proc file that reads and writes in an integer "
	"keept in kernel space");

/*
 * proc_write will write to this value and proc_read will read
 * from this value
 *
 */
int integer_from_user;

/*
 * In this structure we hold information about the /proc file.
 *
 */
struct proc_dir_entry *Proc_File;

/*
 * Arguments
 * ---------
 *
 * 1. File object representing an open file from user space
 * 2. User space buffer
 * 3. Buffer size
 * 4. Requested position (this will be updated)
 *
 * Implementation
 * --------------
 *
 * 1. Check the requested position and the length of buffer
 * 2. Read info from ubuf
 * 3. Return the new position
 *
 */
static ssize_t proc_write(struct file *file, const char __user *ubuf,
	size_t count, loff_t *ppos) {

	printk(KERN_DEBUG "proc_write for /proc/%s was triggered\n", PROCFS_NAME);

	/*
	 * Buffer used for copying data from user space.
	 *
	 */
	char buf[BUFLEN];

	/*
	 * integer_no_read - number of parsed integers from ubuf
	 * from_user	   - temporary variable used to store the integer from user
	 * new_ppos        - length of buffer read from user
	 *
	 */
	int integer_no_read = 0;
	int from_user = 0;
	int new_ppos = 0;

	/*
	 * check if it is the first time we call read (*ppos = 0) and the user
	 * buffer size is at least buflen large.
	 *
	 */
	if (*ppos > 0 || count > BUFLEN) {
		return -EFAULT;
	}

	/*
	 * copy_from_user returns the number of bytes that could not be copied.
	 * On success it returns 0.
	 *
	 */
	if (copy_from_user(buf, ubuf, count)) {
		return -EFAULT;
	}

	/*
	 * Read the integer.
	 *
	 */
	integer_no_read = sscanf(buf, "%d", &from_user);
	if (integer_no_read != 1) {
		return -EFAULT;
	}

	integer_from_user = from_user;

	/*
	 * Update the position.
	 *
	 */
	new_ppos = strlen(buf);
	*ppos = new_ppos;

	return new_ppos;

}

/*
 * Arguments
 * ---------
 *
 * 1. File object representing an open file from user space
 * 2. User space buffer
 * 3. Buffer size
 * 4. Requested position (this will be updated)
 *
 * Implementation
 * --------------
 *
 * 1. Check the requested position
 * 2. Fill the buffer with data from the requested position
 * (be careful not to overflow)
 * 3. Return the number of filled bytes
 *
 */

static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count,
	loff_t *ppos) {

	char buf[BUFLEN];
	int  write_bytes = 0;

	printk(KERN_DEBUG "proc_read for /proc/%s was triggered\n", PROCFS_NAME);

	/*
	 * check if it is the first time we call read (*ppos = 0) and the user
	 * buffer size is at least buflen large.
	 *
	 */

	if (*ppos > 0 || count < BUFLEN) {
		return 0;
	}

	/*
	 * Fill the local buffer and keep track of how many bytes were written.
	 *
	 */

	write_bytes += sprintf(buf, "The integer keept in kernel space is: %d\n", 
		integer_from_user);

	/*
	 * Use copy_to_user for safe transfer to user space.
	 *
	 */

	if (copy_to_user(ubuf, buf, write_bytes)) {
		return -EFAULT;
	}

	/*
	 * Update offset and return the written bytes.
	 *
	 */

	*ppos = write_bytes;

	return write_bytes;

}

/*
 * This structure keeps the operations the proc file can support.
 *
 */
static const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE,
	.read  = proc_read,
	.write = proc_write
};

int init_module() {

	/*
	 * create_proc_entry is deprecated since kernel version 3.10, so we
	 * must use create_proc instead.
	 *
	 * Arguments
	 * ---------
	 *
	 * 1. Char array containing the name of the proc
	 * 2. Permissions of the proc
	 * 3. The name of the parent directory under /proc
	 * 4. Structure in which the file operations for the proc entry will
	 * be created
	 *
	 */
	Proc_File = proc_create(PROCFS_NAME, PERMS, NULL, &proc_file_fops);

	if (Proc_File == 0) {
		return -EPERM;
	}

	printk(KERN_DEBUG "/proc/helloworld created successfully!\n");

	return 0;

}

void cleanup_module() {

	/*
	 * Remove the proc created in init_module.
	 *
	 */
	proc_remove(Proc_File);

	printk(KERN_DEBUG "/proc/helloworld was removed!\n");
}










