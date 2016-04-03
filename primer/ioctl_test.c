#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)

int main( int argc, char *argv[]) {
	
	struct ioctl_test_t {
		char *string;
	} ioctl_test;

	if (argc != 2) {
		printf("Usage: ioctl_test <input>\n");
		return 0;
	}

	int fd = open("/proc/ioctl_test", O_RDONLY);

	ioctl_test.string= argv[1];

	ioctl(fd, IOCTL_TEST, &ioctl_test);

	return 0;
}
