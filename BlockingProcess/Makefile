CONFIG_MODULE_SIG=n

obj-m += sleep.o
user-program += non_block

KERNELDIR=/lib/modules/$(shell uname -r)/build

all:
	# Build the kernel module
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

	# Build the user space program
	$(CC) $(user-program).c -o $(user-program)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f $(user-program)
