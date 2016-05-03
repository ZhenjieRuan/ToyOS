#include "ramdisk.h"

/* filesystem instance */
fs_t* fs;

/* fd table head */
pid_fd_entry_t pid_fd_table[NUM_PID];

int init_fs(uint32_t num_blocks) {
	/* Getting memory for disc*/
	/*int i;*/
	fs = (fs_t *)vmalloc(BLK_SIZE*num_blocks);
	memset(fs, 0, BLK_SIZE*num_blocks);
	printk("<1> sizeof superblock %d\n", sizeof(fs->superblock));
	printk("<1> sizeof inodes region %d\n", sizeof(fs->inodes));
	printk("<1> sizeof bitmap region %d\n", sizeof(fs->bitmap));
	printk("<1> sizeof free blocks region %d\n", sizeof(fs->blocks));

	fs->superblock.freeblocks = NUM_BLOCKS;
	fs->superblock.freeindex = NUM_INODES;

	printk("<1> freeblocks: %d\n", fs->superblock.freeblocks);
	printk("<1> freeinodes: %d\n", fs->superblock.freeindex);	

	/* create root dir */
	mkdir("/");
	print_inode_info(&fs->inodes[0]);

	if (strcmp(fs->inodes[10].type, "") == 0) {
		printk("Empty inode!!!\n");
	}

	if (fs->blocks[1].entries[1].inode_num == 0) {
		printk("Empty dir entry!\n");
	}

	return 0;
}


int create(char* pathname) {
	int len = strlen(pathname);
	int parent = 0, i = 0;
	inode_t *parent_inode, *file_inode;
	dir_entry_t* free_entry_in_parent;
	char *prefix = kmalloc(len, GFP_KERNEL);
	char *filename = kmalloc(len, GFP_KERNEL);
	memset(prefix,0,len);
	memset(filename,0,len);
	/* no more free blocks */
	if (fs->superblock.freeblocks <= 0 || fs->superblock.freeindex <= 0) {
		return -1;
	}

	/* get prefix and filename */
	get_prefix_and_filename(pathname, prefix, filename, len);
	printk("prefix: %s filename: %s\n", prefix, filename);

	/* get parent inode num */
	if ((parent = get_inode_num(fs,prefix)) == -1) {
		printk("<1> The pathname is invalid\n");	
		return -1;
	}

	printk("<1> Parent inode number: %d\n", parent);
	
	parent_inode = &fs->inodes[parent];

	/* get free spot in parent blocks to fill entry */
	if ((free_entry_in_parent = get_free_entry(fs, parent_inode)) == NULL) {
		printk("<1> Error getting free entry spot in parent, parent size:%d\n", parent_inode->size);
		for (i = 0; i < 8; ++i) {
			if (parent_inode->direct_blks[i] != NULL) {
				printk("<1> Block %d:\n", i);
				print_dir_block(parent_inode->direct_blks[i]);
				printk("<1>=================\n");
			}
		}
		return -1;
	}

	if ((file_inode = get_free_inode(fs)) == NULL) {
		printk("<1> Error getting free inode\n");
		return -1;
	}
	printk("<1> Free inode: %d\n", file_inode->num);

	strcpy(free_entry_in_parent->name, filename);
	free_entry_in_parent->inode_num = file_inode->num;

	parent_inode->size += 16;

	strcpy(file_inode->type, "reg\0");


	kfree(prefix);
	kfree(filename);
	return 0;
}

int open(int pid, char* pathname) {
	//Check if it exists
	//create fd need pid and inode
	//set position to zero

	uint16_t inode_num = 0;
	
	/* get parent inode num */
	if ((inode_num = get_inode_num(fs,pathname)) == -1) {
		printk("<1> The pathname is invalid\n");	
		return -1;
	}

	printk("<1> File inode number: %d\n", inode_num);
	
	fd_object_t *fd_object;
	if ((fd_object = create_fd(pid)) == -1) {
		printk("<1> Filled up fd_table.\n");
		return -1;
	}

	// Add inode and set current position to zero
	fd_object->current_pos = 0;
	fd_object->inode = inode_num;

	return fd_object->fd_num;

}

fd_object_t *create_fd(int pid) {

	// Create it
	// return current fd

	// Find current pid entry
	
	fd_table_t *fd_table;
	if((fd_table = get_fd_table(pid)) == -1) {
		printk("<1> Filled up pid_fd_table, allocate more space\n");
		return -1;
	}

	// Scan through fd table and find next availible 
	fd_object_t *fd_object = fd_table;
	int i;
	for (i = 0; i < 1024; i++) {
		if (fd_object[i].used == 0) {
			fd_object[i].fd_num = i;
			fd_object[i].used = 1;
			return &fd_object[i];
		}
	}

	return -1;
}

// returns fd_table corresponding to PID of process


fd_table_t *get_fd_table(int pid) {

	int i;
	for (i = 0; i < NUM_PID; i++) {
		if (pid_fd_table[i].pid == NULL) {
			pid_fd_table[i].pid = pid;
			return &pid_fd_table[i].fd_table;
		}
		else if (pid_fd_table[i].pid == pid) {
			return &pid_fd_table[i].fd_table;
		}
	}


	return -1;
}

// zero out fd_table
void init_fd_table(void) {
	memset(&pid_fd_table, NULL, sizeof(pid_fd_table));
}

int mkdir(char* pathname) {
	inode_t* inode;
	block_t* block;
	
	/* no more free blocks */
	if (fs->superblock.freeblocks <= 0 || fs->superblock.freeindex <= 0) {
		return -1;
	}

	/* creating root dir */
	if (strcmp(pathname, "/") == 0) {
		inode = &fs->inodes[0];
		block = &fs->blocks[0];
		strcpy(inode->type, "dir\0");
		inode->size = 0;
		inode->num = 0;
		inode->direct_blks[0] = block;
		set_bit_of_bitmap(fs->bitmap, 0);
	} else {
	
	}

	fs->superblock.freeblocks--;
	fs->superblock.freeindex--;

	return 0;
}

void cleanup_fs(void) {
	vfree(fs);
}
