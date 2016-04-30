/* 
 * Keyboard Module Demonstration Program
 */
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "discos.h"
	
/* File descriptor to kernel module */
static int fd;

int main() {

	fd = open("/proc/ioctl_dicos_test", O_RDWR);
	
	char *path = "/home";

	ioctl(fd,RD_CREATE, path);
	
	close(fd);

	return 0;
}
