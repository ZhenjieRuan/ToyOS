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

int test_close(int fd, ioctl_args_t* args, int fd_num) {
	args->fd_num = fd_num;
	args->pid = (int)getpid();
	return ioctl(fd, RD_CLOSE, args);
}
//int rd_read(int fd, char *address, int num_bytes)-- read up to num_bytes from a regular file identified by file descriptor, fd, 
//into a process' location at address. You should return the number of bytes actually read, else -1 if there is an error. An 
//error occurs if the value of fd refers either to a non-existent file or a directory file.



int test_read(int fd, ioctl_args_t* args, char *address, int num_bytes, int fd_num) {
	args->fd_num = fd_num; 
	args->r_buffer = address;
	args->num_bytes = num_bytes;
	args->pid = (int)getpid();
	return ioctl(fd, RD_READ, args);
}


// int rd_write(int fd, char *address, int num_bytes) -- write up to num_bytes from the specified address in the calling process 
// to a regular file identified by file descriptor, fd. You should return the actual number of bytes written, or -1 if there is an
//  error. An error occurs if the value of fd refers either to a non-existent file or a directory file.
int test_write(int fd, ioctl_args_t* args, char *address, int num_bytes, int fd_num) {
	args->fd_num = fd_num;
	args->pid = (int)getpid();
	return ioctl(fd, RD_WRITE, args);
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

	int filedesc;
	filedesc = test_open(fd, args);
	printf("fd num = %d\n", filedesc);

	
	ret = test_close(fd, args, filedesc);
	printf("closing %d returns: %d \n", filedesc, ret);

	printf("attempt to close file with madeup fd=77\n");
	ret = test_close(fd, args, 77);
	printf("closing %d returns: %d \n", 77, ret);
	close(fd);

	
	return 0;
}
