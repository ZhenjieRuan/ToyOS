#include "ramdisk.h"

/* filesystem instance */
fs_t* fs;

/* fd table head */
pid_fd_entry_t pid_fd_table[NUM_PID];

int init_fs(uint32_t num_blocks) {
	/* Getting memory for disc*/
	/*int i;*/
	//int test;
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

	/*if (strcmp(fs->inodes[10].type, "") == 0) {*/
		/*printk("<1> Empty inode!!!\n");*/
	/*}*/

	/*if (fs->blocks[1].entries[1].inode_num == 0) {*/
		/*printk("<1> Empty dir entry!\n");*/
	/*}*/


	return 0;
}

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
		printk("<1> Error getting free entry spot in parent, parent size:%d, inode_left: %d\n", parent_inode->size, fs->superblock.freeindex);
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

	/*print_dir_block(get_block_by_num(parent_inode, 0));*/

	kfree(prefix);
	kfree(filename);
	return 0;
}

int create(char* pathname) {
	return create_file("reg\0", pathname);
}




int close(int pid, int fd_num) {
	fd_table_t *fd_table;
	//Use pid to find fd table
	fd_table = get_fd_table(pid);

	if(fd_table == NULL){
		printk("<1> could not find pid in pid_fd_table\n");
		return -1;
	}

	if(	fd_table->fd_object[fd_num].used == 1 ){ //It's being used
		fd_table->fd_object[fd_num].used = 0;
		//This is overkill but keeps state clean.
		fd_table->fd_object[fd_num].current_pos = 0;
		fd_table->fd_object[fd_num].inode = NULL;
		fd_table->fd_object[fd_num].fd_num = 0;
		return 0; // Success
	}

	printk("<1> that fd_num was unused\n");
	return -1; // Error
}

int open(int pid, char* pathname) {
	//Check if it exists
	//create fd need pid and inode
	//set position to zero

	int inode_num = 0;
	fd_object_t *fd_object;

	/* get parent inode num */
	if ((inode_num = get_inode_num(fs,pathname)) == -1) {
		printk("<1> The pathname is invalid\n");	
		return -1;
	}

	printk("<1> File inode number: %d\n", inode_num);
	

	fd_object = create_fd(pid);
	if ( fd_object == NULL) {
		printk("<1> Filled up fd_table.\n");
		return -1;
	}

	// Add inode and set current position to zero
	fd_object->current_pos = 0;
	//fd_object->inode = inode_num;
	fd_object->inode = &fs->inodes[inode_num];

	return fd_object->fd_num;
}

// set current_pos to the new offset
int lseek(int pid, int fd, int offset) {
	printk("<1> at top of lseek. seeking to %d\n",offset);

	fd_table_t *fd_table;
	fd_object_t *fd_object;
	inode_t *inode;

	if (offset < 0 || offset > NUM_BYTES_IN_INODE) {
		printk("<1> Invalid offset %d\n", offset);
		return -1;
	}

	if((fd_table = get_fd_table(pid)) == NULL) {
		printk("<1> Filled up pid_fd_table, allocate more space\n");
		return -1;
	}


	fd_object = (fd_object_t*) fd_table;

	if (fd_object[fd].used != 1) {
		printk("<1> Could not find fd = %d in fd_table.\n", fd);
		return -1;
	}

	// Get pointer to inode
	inode = fd_object[fd].inode;

	if (strcmp(inode->type, "dir\0") == 0) {
		printk("<1> Can't seek a directory file.\n");
		return -1;
	}

	// Check if size greater than offset
	if (inode->size <= offset) {
		fd_object[fd].current_pos = inode->size;
		return fd_object[fd].current_pos;
	}
	printk("<1> current pos is %d\n", fd_object[fd].current_pos);
	// Set file position and return file position
	fd_object[fd].current_pos = offset;
	printk("<1> changed pos to %d\n", fd_object[fd].current_pos);
	return fd_object[fd].current_pos;
}

int readdir(int pid, int fd, char *address) {

	int block_num;
	fd_table_t *fd_table;
	fd_object_t *fd_object;
	inode_t *inode;
	block_t *block;
	dir_entry_t *dir_entry;

	if((fd_table = get_fd_table(pid)) == NULL) {
		printk("<1> Filled up pid_fd_table, allocate more space\n");
		return -1;
	}

	fd_object = (fd_object_t*) fd_table;

	if (fd_object[fd].used != 1) {
		printk("<1> Could not find fd = %d in fd_table.\n", fd);
		return -1;
	}

	// Get pointer to inode
	inode = fd_object[fd].inode;

	if (strcmp(inode->type, "reg\0") == 0) {
		printk("<1> Can't call readdir on the regular file.\n");
		return -1;
	}

	// Check if size greater than offset
	if (inode->size <= fd_object[fd].current_pos) {
		printk("<1> Read all entries.\n");
		return 0;
	}

	// offset -> block_num
	block_num = (fd_object[fd].current_pos / 256);

	// block_num -> block
	if ((block = get_block_by_num(inode, block_num)) == NULL) {
		printk("<1> Could not get block_num = %d for offset = %d\n", block_num, fd_object[fd].current_pos);
		return -1;
	}

	// Typecast as a dir entry
	dir_entry = (dir_entry_t *) block;
	dir_entry += (fd_object[fd].current_pos % 256) / 16;

	// Test if there's any more directory entries
	if (dir_entry == NULL || dir_entry->name == 0) {
		printk("<1> dir_entry is null.\n");
		return -1;
	}

	// Print out to test...
	printk("<1> Directory entry %s -- %d\n", dir_entry->name, dir_entry->inode_num);

	// Advance current_pos
	if (fd_object[fd].current_pos + 16 >= inode->size) {
		fd_object[fd].current_pos = inode->size;
	}
	else {
		fd_object[fd].current_pos += 16;
	}

	// Copy to userspace -- Copies both name and inode number
	if (copy_to_user(address, dir_entry, 16) != 0) {
		printk("<1> Error Copying bytes to the user.\n");
		return -1;
	}

	return 1;
}



fd_object_t *create_fd(int pid) {
	// Create it
	// return current fd
	// Find current pid entry
	
	fd_table_t *fd_table;
	fd_object_t *fd_object;
	int i;

	fd_table = get_fd_table(pid);
	if(fd_table == NULL) {
		printk("<1> Filled up pid_fd_table, allocate more space\n");
		return NULL;
	}

	// Scan through fd table and find next availible 
	fd_object = (fd_object_t*) fd_table;

	for (i = 0; i < 1024; i++) {
		if (fd_object[i].used == 0) {
			fd_object[i].fd_num = i;
			fd_object[i].used = 1;
			return &fd_object[i];
		}
	}
	return NULL;
}

 
//Search pid_fd_table for pid corresponding to caller
//If pid field is 0, we don't have that pid's entry, need 
//To add it. Returns ptr to fd_table corresponding to PID of process
fd_table_t *get_fd_table(int pid) {
	int i;
	for (i = 0; i < NUM_PID; i++) {
		if (pid_fd_table[i].pid == 0) { //Found next empty entry.
			pid_fd_table[i].pid = pid;
			return &pid_fd_table[i].fd_table;
		}
		else if (pid_fd_table[i].pid == pid) { //Found correct entry.
			return &pid_fd_table[i].fd_table;
		}
	}
	return NULL; //Table full & no corresponding entry.
}


// zero out fd_table
void init_fd_table() {
	memset(&pid_fd_table, 0, sizeof(pid_fd_table));
}

//Read from cursor -> < cursor+num_bytes
int read(int fd_num, char *address, int num_bytes, int pid) {
	int i, current_pos, bytes_left, cont_flag, offset, ret;
	int left = num_bytes, flag = 1;
	char *from, *to;
	fd_table_t *fd_table;   //table 
	fd_object_t *fd_object; //object
	inode_t *inode;         //inode
	block_t *block;
	char *kern_buff;


	bytes_left = num_bytes;
	fd_table = get_fd_table(pid);
	if(fd_table == NULL){
		printk("<1> could not find pid in pid_fd_table\n");
		return -1;
	}

	fd_object = &fd_table->fd_object[fd_num];
	inode = fd_object->inode;
	current_pos = fd_object->current_pos;
	//Gives pointer to ith block of data for inode


	//Create kernel buffer
	kern_buff = vmalloc( num_bytes );
	
	//Write data into kern buff.
	//args->num_bytes better be positive

	//BLK_SIZE = 256
	offset = 0;
	cont_flag = 1; //Do we continue? 
	while(cont_flag){
		//Get the block corresponding to cursor
		printk("<1> In read loop, current pos: %d should get block: %d\n", current_pos, current_pos/BLK_SIZE);
		block = get_block_by_num(inode, current_pos / BLK_SIZE);
		if (block == NULL) {
			return -1;
		}

		//Loop at most 256 times.
		for(i=0; i<BLK_SIZE; i++){ //get rid of magic nums
			kern_buff[offset] = block->data[current_pos % BLK_SIZE];
			offset++;
			bytes_left -= 1;
			current_pos += 1;

			if(bytes_left == 0){
				cont_flag = 0; //Done reading
				break;
			}
			if( (current_pos % BLK_SIZE) == 0)
				break; //Past end of block, need to get ptr to next
		}
	}

	from = kern_buff;
	to = address;

	while (flag) {
		if (left > 4096) {
			ret = copy_to_user(to, from, 4096);
			left -= 4096;
			to += 4096;
			from += 4096;
			printk("<1> ret: %d left:%d\n", ret, left);
		} else {
			ret = copy_to_user(to, from, left);
			printk("<1> ret: %d left:%d\n", ret, left);
			flag = 0;
		}
	}

	/*copy_to_user(address, kern_buff, offset);*/
	vfree(kern_buff);
	lseek(pid, fd_num, fd_object->current_pos + offset);
	fd_object->current_pos += offset;
	printk("<1> Read current seek %d\n", fd_object->current_pos);
	return offset;
}

int write(int fd_num, char *address, int num_bytes, int pid) {

	
	int i, current_pos, bytes_left, cont_flag, offset;
	fd_table_t *fd_table;   //table 
	fd_object_t *fd_object; //object
	inode_t *inode;         //inode
	block_t *block;

	printk("<1> in write\n");

	bytes_left = num_bytes;
	fd_table = get_fd_table(pid);
	if(fd_table == NULL){
		printk("<1> could not find pid in pid_fd_table\n");
		return -1;
	}

	if ((fd_object = &fd_table->fd_object[fd_num]) == NULL) {
		printk("<1> Error getting fd object for %d\n", fd_num);
		return -1;
	}
	inode = fd_object->inode;
	current_pos = fd_object->current_pos;
	//Gives pointer to ith block of data for inode

	printk("<1> about to enter write loop, current_pos: %d\n", current_pos);
	//Write data into kern buff.
	//args->num_bytes better be positive

	//BLK_SIZE = 256
	offset = 0;
	cont_flag = 1; //Do we continue? 
	while(cont_flag){
		printk("<1> In write loop, current_pos: %d\n", current_pos);
		//Get the block corresponding to cursor
		block = get_block_by_num(inode, current_pos / BLK_SIZE);
		if(block == NULL){
			printk("<1> got a null block \n");
			block = set_block_by_num(fs, inode, current_pos / BLK_SIZE);
			if (block == NULL) {
				printk("<1> Error allocating free block!!!\n");
				return -1;
			}
		}

		//Loop at most 256 times.
		for(i=0; i<BLK_SIZE; i++){ //get rid of magic nums
			if (block == NULL) {
				printk("<1> Null block\n");
				return -1;
			}
			block->data[current_pos % BLK_SIZE] = address[offset];
			offset +=1;
			bytes_left -= 1;
			current_pos += 1;

			if(bytes_left == 0){
				cont_flag = 0; //Done reading
				break;
			}
			if((current_pos % BLK_SIZE) == 0)
				break; //Past end of block, need to get ptr to next
				}
	}

	int new_position = fd_object->current_pos + offset;
	if(new_position > inode->size){
		inode->size = new_position;
	}
	printk("<1> about to return from write\n");

	lseek(pid, fd_num, new_position);

	return offset;
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
	dir_entry_t* entry_in_parent = NULL;
	int i, inode_num, len = strlen(pathname);
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
		printk("<1> Unlink: Error the parent of file doesn't exist\n");	
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

	/* find the entry of this file in parent */
	for (i = 0; i < NUM_DIRECT_BLK; ++i) {
		if (parent_inode->direct_blks[i] != NULL) {
			if ((entry_in_parent = find_entry_in_block(parent_inode->direct_blks[i], filename)) != NULL) {
				break;
			}
		}
	}

	if (entry_in_parent == NULL && parent_inode->single_indirect != NULL) {
		entry_in_parent = find_entry_in_single_indirect(parent_inode->single_indirect, filename);
	}

	if (entry_in_parent == NULL && parent_inode->double_indirect != NULL) {
		entry_in_parent = find_entry_in_double_indirect(parent_inode->double_indirect, filename);
	}

	if (entry_in_parent == NULL) {
		printk("<1> Unlink: Error entry not found in parent\n");
	}

	/* clear the file info in parent */
	memset(entry_in_parent,0,16);
	parent_inode->size -= 16;
	/* clear the inode */
	memset(file_inode,0,64);
	fs->superblock.freeindex++;

	/*for (i = 0; i < 8; ++i) {*/
		/*if (parent_inode->direct_blks[i] != NULL) {*/
			/*printk("<1> Block %d:\n", i);*/
			/*print_dir_block(parent_inode->direct_blks[i]);*/
			/*printk("<1>=================\n");*/
		/*}*/
	/*}*/

	kfree(prefix);
	kfree(filename);
	return 0;
}

void cleanup_fs(void) {
	vfree(fs);
}
