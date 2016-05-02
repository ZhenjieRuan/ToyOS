ramdisktest:
	$(CC) -m32 -Wall -I../../include -o ramdisk_test ramdisk_test.c
	./ramdisk_test

test:
	$(CC) -m32 -Wall -I../../include -o module_test module_test.c
	./module_test

clean:
	rm ./module_test
	rm ./ramdisk_test
