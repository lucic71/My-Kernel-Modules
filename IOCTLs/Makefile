CONFIG_MODULE_SIG=n

obj-m += chardev.o
ioctl += ioctl

KERNELDIR=/lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

user_proc:
	$(CC) $(ioctl).c -o $(ioctl)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf $(ioctl)
