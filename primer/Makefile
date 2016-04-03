obj-m += test_module.o
obj-m += ioctl_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc ioctl_test.c -o ioctl_test
	insmod ioctl_module.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rmmod ioctl_module
	rm ioctl_test

