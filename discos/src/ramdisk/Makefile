obj-m += ramdisk_moduleko.o
ramdisk_moduleko-objs := file_ops.o util.o ramdisk_module.o

all:
	make -C /usr/src/linux SUBDIRS=$(PWD) modules
	insmod ramdisk_moduleko.ko

clean:
	rmmod ramdisk_moduleko
	make -C /usr/src/linux SUBDIRS=$(PWD) clean
