#include "userspace.h"

	
/* File descriptor to kernel module */
static int fd;

int rd_init() {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	fd = open("/proc/ioctl_ramdisk_test", O_RDWR);
	if (fd == -1) {
		printf("Error open proc entry: %s\n", strerror(errno));
	}
	args->num_blks = NUM_BLKS;

	ret = ioctl(fd, RD_INIT, args);

	/*printf("Init got kernel ret: %d\n", ret);*/


	free(args);
	return ret;
}

int rd_create(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->pathname = pathname;

	ret = ioctl(fd, RD_CREATE, args);

	/*printf("Create got kernel ret: %d\n", ret);*/


	free(args);
	return ret;
}

int rd_mkdir(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->pathname = pathname;

	ret = ioctl(fd, RD_MKDIR, args);

	/*printf("Mkdir got kernel ret: %d\n", ret);*/

	free(args);
	return ret;
}

int rd_open(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->pathname = pathname;

	args->pid = (int)getpid();

	ret = ioctl(fd, RD_OPEN, args);
	/*printf("Open got kernel ret: %d\n", ret);*/

	free(args);
	return ret;
}

int rd_close(int fd_num) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->fd_num = fd_num;

	args->pid = (int)getpid();

	ret = ioctl(fd, RD_CLOSE, args);
	/*printf("Close got kernel ret: %d\n", ret);*/

	free(args);
	return ret;
}

int rd_read(int fd_num, char* address, int num_bytes) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->fd_num = fd_num;	
	args->pid = (int)getpid();
	args->address = address;
	args->num_bytes = num_bytes;

	ret = ioctl(fd, RD_READ, args);
	/*printf("Read got kernel ret: %d\n", ret);*/

	free(args);
	return ret;
}

int rd_write(int fd_num, char* address, int num_bytes) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->fd_num = fd_num;	
	args->pid = (int)getpid();
	args->address = address;
	args->num_bytes = num_bytes;

	ret = ioctl(fd, RD_WRITE, args);
	/*printf("Write got kernel ret: %d\n", ret);*/

	free(args);
	return ret;
}

int rd_lseek(int fd_num, int offset) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	memset(args, 0, sizeof(ioctl_args_t));

	args->pid = (int)getpid();
	args->offset = offset;
	args->fd_num = fd_num;	

	ret = ioctl(fd, RD_LSEEK, args);
	/*printf("Lseek got kernel ret: %d\n", ret);*/

	free(args);
	return ret;
}

int rd_unlink(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	args->pathname = pathname;

	ret = ioctl(fd, RD_UNLINK, args);

	/*printf("Unlink got kernel ret: %d\n", ret);*/

	memset(args, 0, sizeof(ioctl_args_t));

	free(args);
	return ret;
}

int rd_readdir(int fd_num, char *address) {

	int ret;

	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	args->pid = (int)getpid();

	args->fd_num = fd_num;

	args->address = address;

	ret = ioctl(fd, RD_READDIR, args);

	/*printf("readdir got kernel ret: %d\n", ret);*/

	memset(args, 0, sizeof(ioctl_args_t));

	free(args);
	
	return ret;
}
