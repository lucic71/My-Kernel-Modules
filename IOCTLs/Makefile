CONFIG_MODULE_SIG=n

obj-m += chardev.o
ioctl += ioctl

char_dev += char_device
MAJOR += 100

# Loadable Kernel Module
lkm += chardev.ko

KERNELDIR=/lib/modules/$(shell uname -r)/build

all:
	# build the sources for lkm and user space process
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CC) $(ioctl).c -o $(ioctl)

	# insert the module in the kernel and make the node
	insmod $(lkm)
	mknod $(char_dev) c $(MAJOR) 0

clean:
	# clean the files associated with the module and remove
	# the module from the kernel
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rmmod $(lkm)

	# remove the user space executable and the char device
	rm -rf $(ioctl)
	rm -rf $(char_dev)
