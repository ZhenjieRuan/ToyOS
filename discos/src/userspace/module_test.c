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


int test_open(int fd, char *path, ioctl_args_t* args) {

	args->pathname = path;

	args->pid = (int)getpid();

	return ioctl(fd, RD_OPEN, args);
}

int test_lseek(int fd, int fd_num, ioctl_args_t* args) {

	args->pathname = "/file1";

	args->pid = (int)getpid();

	args->offset = 10;

	args->fd_num = fd_num;

	return ioctl(fd, RD_LSEEK, args);

}

int test_readdir(int fd, int fd_num, ioctl_args_t* args) {

	static char name[16];
	int ret;
	int index_node_number;

	memset(name, 0, 16);

	args->pid = (int)getpid();

	args->fd_num = fd_num;

	args->address = name;

	while ((ret = ioctl(fd, RD_READDIR, args)) > 0) {
		printf("ret = %d, filename = %s, inode_num = %u\n", ret, name, *(uint16_t *)(name + 14));
	}

	return ret;

}



int test_close(int fd, ioctl_args_t* args, int fd_num) {
	args->fd_num = fd_num;
	args->pid = (int)getpid();
	return ioctl(fd, RD_CLOSE, args);
}

int main() {
	int ret;
	int fd = open("/proc/ioctl_ramdisk_test", O_RDWR);

	if (fd == -1) {
		printf("Error open proc entry: %s\n", strerror(errno));
	}

	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	test_init(fd, args);

	test_create(fd, args);

	// Testing open
	int filedesc;
	// filedesc = test_open(fd, "/file1", args);
	// printf("fd num = %d\n", filedesc);

	// // Testing lseek
	// int lseek = test_lseek(fd, filedesc, args);
	// printf("Seeked to 10 in a empty file should be 0 = %d\n", lseek);
	
	// // Testing close
	// ret = test_close(fd, args, filedesc);
	// printf("closing %d returns: %d \n", filedesc, ret);

	// printf("attempt to close file with madeup fd=77\n");
	// ret = test_close(fd, args, 77);
	// printf("closing %d returns: %d \n", 77, ret);
	// close(fd);

	// Test readdir
	filedesc = test_open(fd, "/", args);
	ret  = test_readdir(fd, filedesc, args);
	printf("test_readdir returned %d\n", ret);


	return 0;
}
