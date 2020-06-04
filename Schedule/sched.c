#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#define PROC_ENTRY_NAME "sched"
#define WORKQUEUE_NAME  "WQsched.c"

struct proc_dir_entry *proc_file;

/*
 * timer_intrpt   - number of times the timer interrupt has been called so far
 * intrpt_routine - 
 * die            - set to 1 for shutdown
 *
 */

static int timer_intrpt;
static void intrpt_routine(struct work_struct *);

static int die;

/*
 * workq - workqueue structure for this task
 * task  - 
 *
 */

static struct workqueue_struct *workq;

static struct work_struct task;
static DECLARE_WORK(task, intrpt_routine);

/*
 * This function will be called on every timer interrupt.
 *
 */

static void intrpt_routine(struct work_struct *work) {

	timer_intrpt++;

	if (!die)
		queue_delayed_work(workq, &task, 100);

}

static int __init init_sched(void) {

	return 0;

}

static void __exit exit_sched(void) {

}

module_init(init_sched);
module_exit(exit_sched);
