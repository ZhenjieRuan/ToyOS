#include "ramdisk.h"

/* filesystem instance */
fs_t* fs;

int init_fs(uint32_t num_blocks) {
	/* Getting memory for disc*/
	/*int i;*/
	int test;
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
		printk("<1> Empty inode!!!\n");
	}

	if (fs->blocks[1].entries[1].inode_num == 0) {
		printk("<1> Empty dir entry!\n");
	}


	return 0;
}


/*int create(char* pathname) {*/
	/*int len = strlen(pathname);*/
	/*int parent = 0, i = 0;*/
	/*inode_t *parent_inode, *file_inode;*/
	/*dir_entry_t* free_entry_in_parent;*/
	/*char *prefix = kmalloc(len, GFP_KERNEL);*/
	/*char *filename = kmalloc(len, GFP_KERNEL);*/
	/*memset(prefix,0,len);*/
	/*memset(filename,0,len);*/
	/*[> no more free blocks <]*/
	/*if (fs->superblock.freeblocks <= 0 || fs->superblock.freeindex <= 0) {*/
		/*return -1;*/
	/*}*/

	/*[> get prefix and filename <]*/
	/*get_prefix_and_filename(pathname, prefix, filename, len);*/
	/*printk("prefix: %s filename: %s\n", prefix, filename);*/

	/*[> get parent inode num <]*/
	/*if ((parent = get_inode_num(fs,prefix)) == -1) {*/
		/*printk("<1> The pathname is invalid\n");	*/
		/*return -1;*/
	/*}*/

	/*printk("<1> Parent inode number: %d\n", parent);*/
	
	/*parent_inode = &fs->inodes[parent];*/

	/*[> get free spot in parent blocks to fill entry <]*/
	/*if ((free_entry_in_parent = get_free_entry(fs, parent_inode)) == NULL) {*/
		/*printk("<1> Error getting free entry spot in parent, parent size:%d\n", parent_inode->size);*/
		/*for (i = 0; i < 8; ++i) {*/
			/*if (parent_inode->direct_blks[i] != NULL) {*/
				/*printk("<1> Block %d:\n", i);*/
				/*print_dir_block(parent_inode->direct_blks[i]);*/
				/*printk("<1>=================\n");*/
			/*}*/
		/*}*/
		/*return -1;*/
	/*}*/

	/*if ((file_inode = get_free_inode(fs)) == NULL) {*/
		/*printk("<1> Error getting free inode\n");*/
		/*return -1;*/
	/*}*/
	/*printk("<1> Free inode: %d\n", file_inode->num);*/

	/*strcpy(free_entry_in_parent->name, filename);*/
	/*free_entry_in_parent->inode_num = file_inode->num;*/

	/*parent_inode->size += 16;*/

	/*strcpy(file_inode->type, "reg\0");*/


	/*kfree(prefix);*/
	/*kfree(filename);*/
	/*return 0;*/
/*}*/

int create_file(char* type, char* pathname) {
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
	/*printk("prefix: %s filename: %s\n", prefix, filename);*/

	/* get parent inode num */
	if ((parent = get_inode_num(fs,prefix)) == -1) {
		printk("<1> The pathname is invalid\n");	
		return -1;
	}

	/*printk("<1> Parent inode number: %d\n", parent);*/
	
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
	/*printk("<1> Free inode: %d\n", file_inode->num);*/

	strcpy(free_entry_in_parent->name, filename);
	free_entry_in_parent->inode_num = file_inode->num;

	parent_inode->size += 16;

	strcpy(file_inode->type, type);


	kfree(prefix);
	kfree(filename);
	return 0;
}

int create(char* pathname) {
	/*int test;*/
	/*block_t *p1, *p2;*/
	/*p1 = &fs->blocks[10];*/
	/*p2 = &fs->blocks[0];*/

	/*test = p1 - p2;*/
	/*printk("<1> Distance: %d\n", test);*/
	return create_file("reg\0", pathname);
}

int mkdir(char* pathname) {
	inode_t* inode;
	block_t* block;

	/* creating root dir */
	if (strcmp(pathname, "/") == 0) {
		inode = &fs->inodes[0];
		block = &fs->blocks[0];
		strcpy(inode->type, "dir\0");
		inode->size = 0;
		inode->num = 0;
		inode->direct_blks[0] = block;
		set_bit_of_bitmap(fs->bitmap, 0);
		fs->superblock.freeblocks--;
		fs->superblock.freeindex--;
	} else {
		return create_file("dir\0", pathname);
	}

	return 0;
}

int unlink(char* pathname) {
	inode_t *file_inode, *parent_inode;
	int inode_num, len = strlen(pathname);
	char *prefix = kmalloc(len, GFP_KERNEL);
	char *filename = kmalloc(len, GFP_KERNEL);
	memset(prefix,0,len);
	memset(filename,0,len);

	if (strcmp(pathname, "/") == 0) {
		printk("<1> Unlink: Error try to unlink root\n");
		return -1;
	}

	get_prefix_and_filename(pathname, prefix, filename, len);
	printk("prefix: %s filename: %s\n", prefix, filename);

	/* get parent inode num */
	if ((inode_num = get_inode_num(fs,prefix)) == -1) {
		printk("<1> Unlink: Error the file doesn't exist\n");	
		return -1;
	}

	printk("<1> Parent inode number: %d\n", inode_num);
	
	parent_inode = &fs->inodes[inode_num];

	/*inode_num = get_inode_num(fs, pathname);*/

	if ((inode_num = get_inode_num(fs,pathname)) == -1) {
		printk("<1> Unlink: Error the file doesn't exist\n");
		return -1;
	}

	printk("<1> File inode number: %d\n", inode_num);

	file_inode = &fs->inodes[inode_num];

	if (strcmp(file_inode->type, "dir\0") == 0 && check_empty(file_inode) != 0) {
		printk("<1> Unlink: Error try to delete non-empty dir file\n");
	}

	/* clear all blks belongs to this inode, set bitmap */
	clear_inode_content(fs, file_inode);
	/* delete the entry in parent */
	/* clear the inode entry */

	kfree(prefix);
	kfree(filename);
	return 0;
}

void cleanup_fs(void) {
	vfree(fs);
}
