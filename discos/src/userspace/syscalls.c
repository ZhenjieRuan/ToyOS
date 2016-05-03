#include "userspace.h"

	
/* File descriptor to kernel module */
static int fd;

int rd_init() {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));
	fd = open("/proc/ioctl_ramdisk_test", O_RDWR);
	if (fd == -1) {
		printf("Error open proc entry: %s\n", strerror(errno));
	}
	args->num_blks = NUM_BLKS;

	ret = ioctl(fd, RD_INIT, args);

	printf("Init got kernel ret: %d\n", ret);

	memset(args, 0, sizeof(ioctl_args_t));

	return ret;
}

int rd_create(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	args->pathname = pathname;

	ret = ioctl(fd, RD_CREATE, args);

	printf("Create got kernel ret: %d\n", ret);

	memset(args, 0, sizeof(ioctl_args_t));

	return ret;
}

int rd_lseek(int fd_num, int offset) {

	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	args->offset = offset;

	args->pid = (int)getpid();

	// fd for ramdisk and not for ioctl
	args->fd_num = fd_num;

	ret = ioctl(fd, RD_LSEEK, args);

	printf("Lseek got kernel ret: %d\n", ret);

	memset(args, 0, sizeof(ioctl_args_t));

	return ret;
}

int rd_mkdir(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	args->pathname = pathname;

	ret = ioctl(fd, RD_MKDIR, args);

	printf("Mkdir got kernel ret: %d\n", ret);

	memset(args, 0, sizeof(ioctl_args_t));

	return ret;
}

int rd_unlink(char* pathname) {
	int ret;
	ioctl_args_t* args = malloc(sizeof(ioctl_args_t));

	args->pathname = pathname;

	ret = ioctl(fd, RD_UNLINK, args);

	printf("Unlink got kernel ret: %d\n", ret);

	memset(args, 0, sizeof(ioctl_args_t));

	return ret;
}

/*int main() {*/

	/*fd = open("/proc/ioctl_discos_test", O_RDWR);*/
	/*if (fd == -1) {*/
		/*printf("Error open proc entry: %s\n", strerror(errno));*/
	/*}*/
	
	/*close(fd);*/

	/*return 0;*/
/*}*/
