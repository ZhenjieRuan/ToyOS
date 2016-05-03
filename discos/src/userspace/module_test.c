#include "userspace.h"

#define MAX_FILES 524

void test_init(int fd, ioctl_args_t* args) {
	int ret;

	args->num_blks = NUM_BLKS;

	ret = ioctl(fd, RD_INIT, args);

	printf("Got kernel ret: %d\n", ret);

	return;
}

void test_create(int fd, ioctl_args_t* args) {
	static char pathname[80];
	int ret, i;

  for (i = 0; i < 1025; i++) { // go beyond the limit
    sprintf (pathname, "/file%d", i);
		args->pathname = pathname;
		ret = ioctl(fd, RD_CREATE, args);
		if (ret == -1) {
			printf("File creationg failed, created %d files\n", i);
			break;
		}
    memset (pathname, 0, 80);
	}
	
	return;
}


int test_open(int fd, ioctl_args_t* args) {

	args->pathname = "/file1";

	args->pid = (int)getpid();

	return ioctl(fd, RD_OPEN, args);
}


int main() {
	int fd = open("/proc/ioctl_ramdisk_test", O_RDWR);

	if (fd == -1) {
		printf("Error open proc entry: %s\n", strerror(errno));
	}

	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	test_init(fd, args);



	test_create(fd, args);

	int filedesc = test_open(fd, args);

	printf("fd num = %d\n", filedesc);

	
	close(fd);
	return 0;
}
