obj-m += discos_module.o

all:
	make -C /usr/src/linux SUBDIRS=$(PWD) modules
	insmod discos_module.ko
test:
	$(CC) -wall -o discos_test discos_test.c
	./discos_test
clean:
	rmmod discos_module
	make -C /usr/src/linux SUBDIRS=$(PWD) clean
	rm ./discos_test
