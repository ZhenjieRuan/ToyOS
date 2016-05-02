#ifndef _USERSPACE_H_
#define _USERSPACE_H_

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* ramdisk size in bytes */
#define NUM_BLKS 8192

/* magic number used for ioctl */
#define MAGIC_NUM 17

/* macros for ioctl calls  */
#define RD_INIT   _IOR(MAGIC_NUM, 0, int)
#define RD_CREATE _IOR(MAGIC_NUM, 1, char *)

typedef struct ioctl_args {
	int num_blks;
	char* pathname;
} ioctl_args_t;

/* fs operations */
int rd_creat(char *);
int rd_mkdir(char *);
int rd_close(int );
int rd_read(int, char *, int );
int rd_write(int, char *, int);
int rd_lseek(int, int );
int rd_unlink(char *);
int rd_readdir(int, char *);

#endif
