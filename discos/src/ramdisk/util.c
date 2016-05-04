/*
 * Utility functions.
 * bitmap memory interface. lets you treat the bitmap as
 * a bit addressable data structure.
 */
#include "ramdisk.h"


/* bitmap interface */
int next_zero_bit(unsigned char ch){
  //find index of next unset bit in byte.
  int i;
  for(i=0; i<8; i++) {
    if((ch & (unsigned char) 1) == 0) {
      return (int)i;
    }
    ch = ch >> 1;
  }
  return -1;
}

int next_zero_bitmap(unsigned char *map, int len){
  //find index of next unset bit in bitmap.
  int i, bit;
  bit = 0;
  for(i=0; i<len; i++){
    if(map[i] == 255){
      bit += 8;
    }else {
      return bit + next_zero_bit(map[i]);
    }
  }
  return -1;
}

void set_bit_of_byte(unsigned char *ch, int len){
  //set bit at offset len.
  *ch = *ch | (1 << len);
}

void set_bit_of_bitmap(unsigned char *map, int len) {
  //assert bit at offset len.
  int byte = len / 8;
  int bit = len % 8;
  set_bit_of_byte(&map[byte], bit);
}

void unset_bit_of_byte(unsigned char *ch, unsigned char len) {
  //deassert bit at offset len.
  *ch = *ch & ~(1 << len);
}

void unset_bit_of_bitmap(unsigned char *map, int len) {
  //deassert bit at offset len.
  int byte = len / 8;
  int bit = len % 8;
  unset_bit_of_byte(&map[byte], bit);
}

void set_next_free_bit(unsigned char *map) {
  //some error checking would be nice.
  set_bit_of_bitmap(map, next_zero_bitmap( map, 3));
}

/* print out directory block content */
void print_dir_block(block_t* block) {
	int i = 0;
	dir_entry_t* entry;
	for (i = 0; i < BLK_SIZE/16; ++i) {
		entry = &block->entries[i];
		if (entry != NULL) {
			printk("<1> Name: %s inode_num: %u\n", entry->name, entry->inode_num);
		} else {
			printk("<1> NULL\n");
		}
	}
}

/* print out inode info */
void print_inode_info(inode_t* inode) {
	printk("<1> Num: %d\n", inode->num);
	printk("<1> Type: %s\n", inode->type);
	printk("<1> Size: %d\n", inode->size);
}

/* return parent inode number if pathname is valid, -1 on error
 * Error Condition:
 * Pathname prefix refers to a non-existent directory
 */
int get_inode_num(fs_t* fs, char* pathname) {
	int i;
	uint16_t found = 0;
	char *token, *string, *tofree;
	block_t* current_blk;
	inode_t* current_inode;
	dir_entry_t* entry;
	tofree = string = kstrdup(pathname, GFP_KERNEL);
	if (string != NULL) {

		/* root dir, return immediately */
		if (strcmp(pathname, "/") == 0) {
			return 0;
		}

		/* skip the root dir */
		string++;
		/* starting search from the root dir */
		current_inode = &fs->inodes[0];

		while ((token = strsep(&string,"/")) != NULL) {
			printk("Locating: %s\n", token);
			/* regular file can't be used as parent dir */
			if (strcmp(current_inode->type, "reg") == 0) {
				printk("<1> Error get inode num: Treat reg file as dir file\n");
				return -1;
			}

			printk("<1> Searching in inode %d\n", current_inode->num);
			/* walk through the direct blocks */
			for (i = 0; i < NUM_DIRECT_BLK; ++i) {
				current_blk = current_inode->direct_blks[i];
				/*print_dir_block(current_blk);*/
				if (current_blk != NULL) {
					if ((entry = find_entry_in_block(current_blk, token)) != NULL) {
						found = entry->inode_num;
						break;
					}
				}
			}

			if (current_inode->single_indirect == NULL) {
				printk("<1> NULL single indirect!\n");
			}

			/* walk through single-indirect blocks */
			if (found == 0 && current_inode->single_indirect != NULL) {
				printk("<1> Search %s in single indirect\n", token);
				if ((entry = find_entry_in_single_indirect(current_inode->single_indirect, token)) != NULL) {
					found = entry->inode_num;
				}
			}

			/* walk through double-indirect blocks */
			if (found == 0 && current_inode->double_indirect != NULL) {
				printk("<1> Search %s in double indirect\n", token);
				if ((entry = find_entry_in_double_indirect(current_inode->double_indirect, token)) != NULL) {
					found = entry->inode_num;
				}
			}

			/* walked all block of current inode, file not found */
			if (found == 0) {
				return -1;
			}

			current_inode = &fs->inodes[found];
		}
	}

	return found;
}

/* search dir block for certain dir, inode number if found, 0 if not found */
dir_entry_t* find_entry_in_block(block_t* block, char* pathname) {
	dir_entry_t* entry;
	int i;
	for (i = 0; i < BLK_SIZE/16; ++i) {
		entry = &block->entries[i];
		if (entry != NULL && strcmp(entry->name, pathname) == 0) {
			return entry;
		}
	}
	return NULL;
}

dir_entry_t* find_entry_in_single_indirect(single_indirect_t* si_blk, char* pathname) {
	block_t* current_blk;
	int i;
	dir_entry_t* entry;
	for (i = 0; i < BLK_SIZE/4; ++i) {
		current_blk = si_blk->blocks[i];
		if (current_blk != NULL) {
			if ((entry = find_entry_in_block(current_blk,pathname)) != NULL) {
				return entry;
			}
		}
	}
	return NULL;
}

dir_entry_t* find_entry_in_double_indirect(double_indirect_t* di_blk, char* pathname) {
	single_indirect_t* current_si_blk;
	int i;
	dir_entry_t* entry;
	for (i = 0; i < BLK_SIZE/4; ++i) {
		current_si_blk = di_blk->blocks[i];
		if (current_si_blk != NULL) {
			if ((entry = find_entry_in_single_indirect(current_si_blk, pathname)) != NULL) {
				return entry;
			}
		}
	}
	return NULL;
}


/* seperate prefix and filename */
void get_prefix_and_filename(char* pathname, char* prefix, char* filename, int len) {
	int i, cutoff = 0;
	for (i = len - 1; i >= 0; --i) {
		if (pathname[i] == '/') {
			cutoff = i;
			break;
		}
	}
	if (cutoff == 0) {
		strcpy(prefix,"/\0");
	} else {
		strncpy(prefix, pathname, cutoff);
	}
	strcpy(filename, &pathname[cutoff+1]);
}

/* get free entry in dir */
dir_entry_t* get_free_entry(fs_t* fs, inode_t* parent) {
	int i, region = parent->size / BLK_SIZE;
	block_t* current_blk;
	dir_entry_t* free_entry;
	if (region < 8) {
		/* there is still space in direct blocks */
		for (i = region; i < NUM_DIRECT_BLK; ++i) {	
			if (parent->direct_blks[i] == NULL) {
				/* getting a new block */
				if ((parent->direct_blks[i] = get_free_block(fs)) == NULL) {
					printk("<1> No more free blocks\n");
					return NULL;
				}
				return &parent->direct_blks[i]->entries[0];
			}
			current_blk = parent->direct_blks[i];
			free_entry = get_free_entry_direct_block(current_blk);
			if (free_entry != NULL) return free_entry;
		}
	} else if (region < 72) {
		/* free spot in single indirect */
		return get_free_entry_single_indirect(fs,parent, region - 8);
	} else if (region < 4168) {
		/* free spot in double indirect */
		return get_free_entry_double_indirect(fs,parent, region - 72);
	}
	return NULL;
}

dir_entry_t* get_free_entry_direct_block(block_t* block) {
	int i;
	/*dir_entry_t* current_entry;*/
	for (i = 0; i < BLK_SIZE/16; ++i) {
		if (block->entries[i].inode_num == 0) {
			return &block->entries[i];
		}
	}
	return NULL;
}

dir_entry_t* get_free_entry_single_indirect(fs_t* fs, inode_t* parent, int region) {
	block_t* current_blk;
	single_indirect_t* si_blk = parent->single_indirect;
	if (si_blk == NULL) {
		if ((si_blk = (single_indirect_t *)get_free_block(fs)) == NULL) {
			printk("<1> No more free blocks\n");
			return NULL;
		}
		parent->single_indirect = si_blk;
	}
	current_blk = si_blk->blocks[region];	
	if (current_blk == NULL) {
		if ((si_blk->blocks[region] = get_free_block(fs)) == NULL) {
				printk("<1> No more free blocks\n");
				return NULL;
		}
		return &si_blk->blocks[region]->entries[0];
	}
	return get_free_entry_direct_block(current_blk);
}

dir_entry_t* get_free_entry_double_indirect(fs_t* fs, inode_t* parent , int region) {
	single_indirect_t* si_blk;
	double_indirect_t* di_blk = parent->double_indirect;
	if (di_blk == NULL) {
		if ((di_blk = (double_indirect_t *)get_free_block(fs)) == NULL) {
			printk("<1> No more free blocks\n");
			return NULL;
		}
		parent->double_indirect = di_blk;
	}
	si_blk = di_blk->blocks[region/64];
	return get_free_entry_single_indirect(fs, parent, region%64);
}

/* get free inode */
inode_t* get_free_inode(fs_t* fs) {
	inode_t* inode;
	int i;
	for (i = 0; i < NUM_INODES; ++i) {
		if (strcmp(fs->inodes[i].type, "") == 0) {
			/* this inode is not yet used */
			inode = &fs->inodes[i];
			inode->num = i;
			fs->superblock.freeindex--;
			return inode;
		}
	}
	return NULL;
}

/* get free block */
block_t* get_free_block(fs_t* fs) {
	int block_index = next_zero_bitmap(fs->bitmap, 4*BLK_SIZE);
	if (block_index < NUM_BLOCKS && fs->superblock.freeblocks > 0) {
		fs->superblock.freeblocks--;
		set_bit_of_bitmap(fs->bitmap, block_index);
		memset(&fs->blocks[block_index],0,BLK_SIZE);
		return &fs->blocks[block_index];
	}
	return NULL;
}

int check_empty_block(block_t* block) {
	int i;
	for (i = 0; i < BLK_SIZE; ++i) {
		if (block->data[i] != 0)
			return -1;
	}
	return 0;
}

int check_empty_single_indirect(single_indirect_t* si_blk) {
	int i;
	for (i = 0; i < BLK_SIZE/4; ++i) {
		if (si_blk->blocks[i] != NULL) {
			if (check_empty_block(si_blk->blocks[i]) != 0)
				return -1;
		}
	}
	return 0;
}

int check_empty_double_indirect(double_indirect_t* di_blk) {
	int i;
	for (i = 0; i < BLK_SIZE/4; ++i) {
		if (di_blk->blocks[i] != NULL) {
			if (check_empty_single_indirect(di_blk->blocks[i]) != 0)
				return -1;
		}
	}
	return 0;
}


/* check if all block of inode is empty, 0 if empty, -1 otherwise */
int check_empty(inode_t* inode) {
	int i = 0;
	for (i = 0; i < NUM_DIRECT_BLK; ++i) {
		if (inode->direct_blks[i] != NULL && check_empty_block(inode->direct_blks[i]) != 0)
			return -1;
	}
	if (inode->single_indirect != NULL && check_empty_single_indirect(inode->single_indirect) != 0)
		return -1;
	if (inode->double_indirect != NULL && check_empty_double_indirect(inode->double_indirect) != 0)
		return -1;
	return 0;
}

/* clear the content of a single indirect block */
void clear_single_indirect(fs_t* fs, single_indirect_t* si_blk) {
	int i;
	block_t* start = &fs->blocks[0];
	for (i = 0; i < BLK_SIZE/4; ++i) {
		if (si_blk->blocks[i] != NULL) {
			unset_bit_of_bitmap(fs->bitmap, si_blk->blocks[i] - start);
			memset(si_blk->blocks[i], 0, BLK_SIZE);
			fs->superblock.freeblocks++;
		}
	}
	unset_bit_of_bitmap(fs->bitmap, (block_t*)si_blk - start);
	memset(si_blk, 0, BLK_SIZE);
	fs->superblock.freeblocks++;
}

/* clear the content of a double indirect block */
void clear_double_indirect(fs_t* fs, double_indirect_t* di_blk) {
	int i;
	block_t* start = &fs->blocks[0];
	for (i = 0; i < BLK_SIZE/4; ++i) {
		if (di_blk->blocks[i] != NULL) {
			clear_single_indirect(fs,di_blk->blocks[i]);
		}
	}
	unset_bit_of_bitmap(fs->bitmap, (block_t*)di_blk - start);
	memset(di_blk, 0, BLK_SIZE);
	fs->superblock.freeblocks++;
}

/* clear the content of a inode */
void clear_inode_content(fs_t* fs, inode_t* inode) {
	int i;
	block_t *start, *current_blk;
	start = &fs->blocks[0];
	for (i = 0; i < NUM_DIRECT_BLK; ++i) {
		current_blk = inode->direct_blks[i];
		if (current_blk != NULL) {
			unset_bit_of_bitmap(fs->bitmap, inode->direct_blks[i] - start);
			memset(inode->direct_blks[i], 0, BLK_SIZE);
			fs->superblock.freeblocks++;
		}
	}
	if (inode->single_indirect != NULL) {
		clear_single_indirect(fs,inode->single_indirect);
	}
	if (inode->double_indirect != NULL) {
		clear_double_indirect(fs,inode->double_indirect);
	}
}

block_t* get_block_by_num(inode_t* inode, int block_num) {
	if (inode == NULL) {
		return NULL;
	}
	if (block_num < 8) {
		return inode->direct_blks[block_num];
	} else if (block_num < 72) {
		if (inode->single_indirect != NULL) {
			return inode->single_indirect->blocks[block_num - 8];
		}
	} else if (block_num < 4168) {
		if (inode->double_indirect != NULL) {
			if (inode->double_indirect->blocks[(block_num - 72)/64] != NULL) {
				return inode->double_indirect->blocks[(block_num - 72)/64]->blocks[(block_num - 72)%64];
			}
		} else {
			printk("<1> Null double indirect block!!!\n");
		}
	}
	return NULL;
}

block_t* set_block_by_num(fs_t* fs, inode_t* inode, int block_num) {
	block_t* new_blk;
	if (inode == NULL) {
		return NULL;
	}
	if ((new_blk = get_free_block(fs)) == NULL) {
		printk("<1> No more free blocks\n");
		return NULL;
	}
	if (block_num < 8) {
		inode->direct_blks[block_num] = new_blk;
		/*return new_blk;*/
	} else if (block_num < 72) {
		if (inode->single_indirect != NULL) {
			inode->single_indirect->blocks[block_num - 8] = new_blk;
		} else {
			inode->single_indirect = (single_indirect_t*)new_blk;
			if ((new_blk = get_free_block(fs)) == NULL) {
				printk("<1> No more free blocks\n");
				return NULL;
			}
			inode->single_indirect->blocks[0] = new_blk;
		}
	} else if (block_num < 4168) {
		if (inode->double_indirect != NULL) {
			if (inode->double_indirect->blocks[(block_num - 72)/64] != NULL) {
				inode->double_indirect->blocks[(block_num - 72)/64]->blocks[(block_num - 72)%64] = new_blk;
			} else {
				inode->double_indirect->blocks[(block_num - 72)/64] = new_blk;
				if ((new_blk = get_free_block(fs)) == NULL) {
					printk("<1> No more free blocks\n");
					return NULL;
				}
				inode->double_indirect->blocks[(block_num - 72)/64]->blocks[0] = new_blk;
			}
		} else {
			inode->double_indirect = (double_indirect_t*)new_blk;
			if ((new_blk = get_free_block(fs)) == NULL) {
				printk("<1> No more free blocks\n");
				return NULL;
			}
			inode->double_indirect->blocks[0] = new_blk;
			if ((new_blk = get_free_block(fs)) == NULL) {
				printk("<1> No more free blocks\n");
				return NULL;
			}
			inode->double_indirect->blocks[0]->blocks[0] = new_blk;
		}
	}
	return new_blk;
}










