#include "ramdisk.h"

/* filesystem instance */
fs_t* fs;

int init_fs(uint32_t num_blocks) {
	/* Getting memory for disc*/
	fs = (fs_t *)vmalloc(BLK_SIZE*num_blocks);
	printk("<1> sizeof superblock %d\n", sizeof(fs->superblock));
	printk("<1> sizeof inodes region %d\n", sizeof(fs->inodes));
	printk("<1> sizeof bitmap region %d\n", sizeof(fs->bitmap));
	printk("<1> sizeof free blocks region %d\n", sizeof(fs->blocks));

	fs->superblock.freeblocks = NUM_BLOCKS;
	fs->superblock.freeindex = NUM_INODES;

	/* create root dir */
	mkdir("/");

	return 0;
}


int create(char* pathname) {
	/* no more free blocks */
	if (fs->superblock.freeblocks <= 0 || fs->superblock.freeindex <= 0) {
		return -1;
	}

	return 0;
}

int mkdir(char* pathname) {
	/* no more free blocks */
	if (fs->superblock.freeblocks <= 0 || fs->superblock.freeindex <= 0) {
		return -1;
	}

	inode_t* inode;
	block_t* block;
	if (strcmp(pathname, "/") == 0) {
		inode = fs->inodes[0];
		block = fs->blocks[0];
		inode->type = "dir\0";
		inode->direct_blks[0] = block;
		inode->size = 0;
	} else {
	
	}

	fs->superblock.freeblocks--;
	fs->superblock.freeindex--;

	return 0;
}

void cleanup_fs(void) {
	vfree(fs);
}
