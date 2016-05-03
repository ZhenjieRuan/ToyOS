# Makefile DISCOS 2016
# Jason Ruan, Sean Smith, Tommy Unger
# just hit `make`!

all: clean userspace_compile run

ramdisktest:
	$(CC) -m32 -Wall -I../../include -o ramdisk_test ramdisk_test.c syscalls.c
	./ramdisk_test

test: userspace_compile run

userspace_compile:
	$(CC) -m32 -Wall -I../../include -o module_test module_test.c

run:
	./module_test

kernel_compile:
	make -C ../ramdisk

clean:
	-rm ./module_test
	-rm ./ramdisk_test
