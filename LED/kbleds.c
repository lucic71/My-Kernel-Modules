#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>

// I could not find fg_console in any header file so I exported
// it from drivers/tty/vt/vt.c
extern int fg_console;

MODULE_DESCRIPTION("Illustrates the ues of keyboard LEDs");
MODULE_AUTHOR("Lucian");
MODULE_LICENSE("GPL");

struct timer_list timer;
struct tty_driver *driver;
char kbled_status;

// HZ is a compile-time constant which defines the timer interrrupt rate
#define BLINK_DELAY HZ/5

// see man page of ioctl_console
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF

/*
 * Arguments
 * ---------
 *  1. Pointer to the argument of the IOCTL.
 *
 * Description
 * -----------
 *  This function blinks the keyboard LEDS periodically by invoking the KDSETLED
 *  ioctl on the keyboard driver.
 *
 *  The argument to KDSETLED are ALL_LEDS_ON (causing the led mode to be set to
 *  LED_SHOW_IOCTL, and all leds are lit) and the RESTORE_LED (any value grater
 *  than 0x07 switches back the led mode to LED_SHOW_FLAGS, thus the leds reflect
 *  the actual keyboard status).
 *
 */

static void LED_timer_setter(unsigned long ptr) {

	int *pstatus = (int *) ptr;

	if (*pstatus == ALL_LEDS_ON) {
		*pstatus = RESTORE_LEDS;
	} else {
		*pstatus = ALL_LEDS_ON;
	}

	(driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);

	timer.expires = jiffies + BLINK_DELAY;
	add_timer(&timer);

}

/*
 * Description
 * -----------
 *
 *  For debugging purposes I will scan the consoles until I found one that
 *  has no data inside it. I saw that only the consoles that have 
 *  the index 'i' lesser that fg_console have valid data inside them.
 *  (Cannot explain why at the moment)
 *
 *  After that we need to extract the driver for fg_console.
 *
 *  Next we will set the timer using the timer_list data structure.
 *  It must contain a function, the data and the expiration time expressed
 *  in jiffies (jiffies is just a unit of time dependant on HZ)
 *
 */

static int __init kbleds_init(void) {

	int i;

	printk(KERN_DEBUG "kbleds: fg_console is %x\n", fg_console);

	for (i = 0; i < MAX_NR_CONSOLES; i++) {

		if (!vc_cons[i].d) {
			break;
		}

		printk(KERN_DEBUG "kbleds: console[%i, %i], #%i, tty %lx\n",
			i, MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
			(unsigned long) vc_cons[i].d->port.tty);

	}

	driver = vc_cons[fg_console].d->port.tty->driver;

	init_timer(&timer);

	timer.function = LED_timer_setter;
	timer.data = (unsigned long) &kbled_status;
	timer.expires = jiffies + BLINK_DELAY;

	add_timer(&timer);
	printk(KERN_DEBUG "kbleds: added timer\n");

	return 0;

}

/*
 * Description
 * -----------
 *
 *  Delete the timer and restore the LED state using the IOCTL inside
 *  the keyboard driver.
 *
 */

static void __exit kbleds_exit(void) {

	del_timer(&timer);
	(driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);

	printk("kbleds: removing the module");
}

module_init(kbleds_init);
module_exit(kbleds_exit);
