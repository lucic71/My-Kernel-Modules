# module signature
CONFIG_MODULE_SIG=n

# objects to be compiled
obj-m += sched.o

KERNELDIR=/lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
