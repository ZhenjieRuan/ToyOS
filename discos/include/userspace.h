#ifndef _USERSPACE_H_
#define _USERSPACE_H_

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/* ramdisk num blocks */
#define NUM_BLKS 8192

/* magic number used for ioctl */
#define MAGIC_NUM 17

/* macros for ioctl calls  */
#define RD_INIT   _IOR(MAGIC_NUM, 0, ioctl_args_t*)
#define RD_CREATE _IOR(MAGIC_NUM, 1, ioctl_args_t*)
#define RD_MKDIR  _IOR(MAGIC_NUM, 2, ioctl_args_t*)
#define RD_OPEN   _IOR(MAGIC_NUM, 3, ioctl_args_t*)
#define RD_CLOSE  _IOR(MAGIC_NUM, 4, ioctl_args_t*)
#define RD_READ   _IOR(MAGIC_NUM, 5, ioctl_args_t*)
#define RD_WRITE  _IOR(MAGIC_NUM, 6, ioctl_args_t*)
#define RD_LSEEK  _IOR(MAGIC_NUM, 7, ioctl_args_t*)
#define RD_UNLINK _IOR(MAGIC_NUM, 8, ioctl_args_t*)
#define RD_READDIR _IOR(MAGIC_NUM, 9, ioctl_args_t*)

typedef struct ioctl_args {
	int num_bytes;  //size of read
	int num_blks;
	int pid;
	int offset;
	int fd_num;
	char *address;
	char* pathname;
} ioctl_args_t;

/* fs operations */
int rd_init();
int rd_create(char *);
int rd_mkdir(char *);
int rd_open(char *);
int rd_close(int );
int rd_read(int, char *, int );
int rd_write(int, char *, int);
int rd_lseek(int, int );
int rd_unlink(char *);
int rd_readdir(int, char *);

#endif
