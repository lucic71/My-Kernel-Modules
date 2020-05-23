#include <linux/module.h>
//#include <linux/config.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>

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


}

static int __init kbleds_init(void) {

	int i;

	/*
	 * MAX_NR_CONSOLES is defined in vt.h
	 * vc_cons in defined in vt.c
	 *
	 * This for loop scans the consoles.
	 *
	 */

	for (i = 0; i < MAX_NR_CONSOLES; i++) {

		if (!vc_cons[i].d) {
			break;
		}

	}

	// Get the driver for the current foreground console.
	driver = vc_cons[fg_console].d->vc_tty->driver;

	// Set the LED blink timer.
	init_timer(&timer);


}

static void __exit kbleds_exit(void) {

}

module_init(kbleds_init);
module_exit(kbleds_exit);
