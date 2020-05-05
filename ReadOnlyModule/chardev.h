#ifndef CHARDEV_H_
#define CHARDEV_H_

/*
 * Functions for initialization and cleanup of the module.
 *
 */
int  init_module(void);
void cleanup_module(void);

/*
 * Functions used in struct file_operations.
 *
 */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);

static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#endif
