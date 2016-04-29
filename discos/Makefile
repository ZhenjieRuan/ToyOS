obj-m += keyboard_module.o

all:
	make -C /usr/src/kernels/$(shell uname -r) SUBDIRS=$(PWD) modules
	sudo insmod keyboard_module.ko
demo:
	$(CC) -Wl,-wrap,getchar -o keyboard_demo keyboard_test.c
	./keyboard_demo
clean:
	sudo rmmod keyboard_module
	make -C /usr/src/kernels/$(shell uname -r) SUBDIRS=$(PWD) clean
	rm ./keyboard_demo