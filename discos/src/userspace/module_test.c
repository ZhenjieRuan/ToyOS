#include "userspace.h"

void test_init(int fd, ioctl_args_t* args) {
	int ret;

	args->num_blks = NUM_BLKS;

	ret = ioctl(fd, RD_INIT, args);

	printf("Got kernel ret: %d\n", ret);

	return;
}

void test_create(int fd, ioctl_args_t* args) {
	int ret;

	args->pathname = "/file1";

	ret = ioctl(fd, RD_CREATE, args);

	printf("Got kernel ret: %d\n", ret);
	
	return;
}

int main() {
	int fd = open("/proc/ioctl_ramdisk_test", O_RDWR);

	if (fd == -1) {
		printf("Error open proc entry: %s\n", strerror(errno));
	}

	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	test_init(fd, args);

	test_create(fd, args);
	
	close(fd);
	return 0;
}
