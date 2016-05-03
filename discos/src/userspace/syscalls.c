/* 
 * Keyboard Module Demonstration Program
 */
#include "userspace.h"

	
/* File descriptor to kernel module */
static int fd;

int rd_create(char* pathname) {
	ioctl(fd,RD_CREATE, pathname);
	return 0;
}


int rd_open(char* pathname) {
	return ioctl(fd, RD_OPEN, (int)getpid(), pathname);
}


int main() {

	fd = open("/proc/ioctl_discos_test", O_RDWR);
	if (fd == -1) {
		printf("Error open proc entry: %s\n", strerror(errno));
	}
	
	close(fd);

	return 0;
}
