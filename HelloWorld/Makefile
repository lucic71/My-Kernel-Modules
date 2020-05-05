# module signature
CONFIG_MODULE_SIG=n

# objects to be compiled
obj-m += hello-1.o
obj-m += hello-2.o
obj-m += hello-3.o
obj-m += startstop.o
startstop-objs := start.o stop.o

KERNELDIR=/lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
