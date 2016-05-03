#ifndef _RAMDISK_H_
#define _RAMDISK_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ioctl.h>

/* magic number used for ioctl */
#define MAGIC_NUM 17

/* macros for ioctl calls  */
#define RD_INIT   _IOR(MAGIC_NUM, 0, int)
#define RD_CREATE _IOR(MAGIC_NUM, 1, char *)

typedef struct ioctl_args {
	int num_blks;
	char* pathname;
} ioctl_args_t;

/* fs constants */
#define BLK_SIZE 256
#define BITMAP_BLOCKS 4
#define NUM_BLOCKS 7931
#define NUM_DIRECT_BLK 8
#define NUM_INODES 1024

typedef struct dir_entry {
	char name[14];
	uint16_t inode_num;
}__attribute__((packed)) dir_entry_t;

typedef union block {
	char data[BLK_SIZE];
	dir_entry_t entries[BLK_SIZE/16];
} block_t;

/* block for single indirect */
typedef struct single_indirect {
	block_t *blocks[BLK_SIZE/4];
} single_indirect_t;

/* block for double indirect */
typedef struct double_indirect {
	single_indirect_t *blocks[BLK_SIZE/4];
} double_indirect_t;


/**
 * should name the struct to inode otherwise will conflict with
 * linux/fs definition
 */
typedef struct my_inode { 
	char type[4];
	int size;
	int num;
	block_t *direct_blks[NUM_DIRECT_BLK];
	single_indirect_t *single_indirect;
	double_indirect_t *double_indirect;
	char unused[12];
} inode_t;

typedef struct superblock {
	int freeblocks; /* free block remaining */
	int freeindex; /* free inodes remaining */
	/* free space */
	char unused[248];
} superblock_t;

typedef struct fs {
  //256
  superblock_t superblock;
  //256^2 = 64*1024
	inode_t inodes[NUM_INODES];
  //256*4 
  char bitmap[BITMAP_BLOCKS * BLK_SIZE];
  //2^21 -66816 =  2^13 -261
  block_t blocks[NUM_BLOCKS];
} fs_t;

/* ======================= FD table stuff ======================= */

// current position
// inode pointer
typedef struct fd_object {
	int current_pos;
	inode_t *inode;
} fd_object_t;

// table contains fd objects
typedef struct fd_table {
	fd_object_t fd_object[1024];
} fd_table_t;

typedef struct pid_fd_entry {
	int pid;
	fd_table_t *fd_table;
} pid_fd_entry_t;

// Contains the PID -> fd_table mapping
typedef struct pid_to_fd_table {
	pid_fd_entry_t pid_fd_entry[5];
} pid_to_fd_table_t;

/* ============================================================= */

/* bitmap interfaces */
int next_zero_bitmap(unsigned char*, int); /* find next unset bit in bitmap */
void set_bit_of_bitmap(unsigned char*, int); /* assert bit at offset len */
void unset_bit_of_bitmap(unsigned char *, int); /* deassert bit at offset len */

/* util functions */
inode_t* get_free_inode(fs_t* fs);
int get_inode_num(fs_t*, char *);
void get_prefix_and_filename(char* pathname, char* prefix, char* filename, int len);
dir_entry_t* get_free_entry(fs_t* fs, inode_t*);

/* printing functions for debugging */
void print_dir_block(block_t *);
void print_inode_info(inode_t *);

/* filesystem operations */
int init_fs(uint32_t);
int create(char *);
int mkdir(char *);
void cleanup_fs(void);

#endif /* ifndef _RAMDISK_H_ */
