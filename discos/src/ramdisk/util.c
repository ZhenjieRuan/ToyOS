/*
 * Utility functions.
 * bitmap memory interface. lets you treat the bitmap as
 * a bit addressable data structure.
 */
#include "ramdisk.h"

uint16_t find_entry_in_block(block_t*, char *);
uint16_t find_entry_in_single_indirect(single_indirect_t *, char *);
uint16_t find_entry_in_double_indirect(double_indirect_t *, char *);
dir_entry_t* get_free_entry_direct_block(block_t*);
/*dir_entry_t* get_free_spot_single_indirect(single_indirect_t*);*/
/*dir_entry_t* get_free_spot_double_indirect(double_indirect_t*);*/

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
 * 1. Pathname prefix refers to a non-existent directory
 * 2. Try to create existing file
 */
int get_inode_num(fs_t* fs, char* pathname) {
	int i;
	uint16_t found = 0;
	char *token, *string, *tofree;
	block_t* current_blk;
	inode_t* current_inode;
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

			/* walk through the direct blocks */
			for (i = 0; i < NUM_DIRECT_BLK; ++i) {
				current_blk = current_inode->direct_blks[i];
				if (current_blk != NULL) {
					if ((found = find_entry_in_block(current_blk, pathname)) != 0) {
						break;
					}
				}
			}

			/* walk through single-indirect blocks */
			if (!found) {
				found = find_entry_in_single_indirect(current_inode->single_indirect, pathname);
			}

			/* walk through double-indirect blocks */
			if (!found) {
				found = find_entry_in_double_indirect(current_inode->double_indirect, pathname);
			}

			/* walked all block of current inode, file not found */
			if (!found) {
				return -1;
			}

			current_inode = &fs->inodes[found];

		}
	}

	return found;
}

/* search dir block for certain dir, inode number if found, 0 if not found */
uint16_t find_entry_in_block(block_t* block, char* pathname) {
	dir_entry_t* entry;
	int i;
	for (i = 0; i < BLK_SIZE/16; ++i) {
		entry = &block->entries[i];
		if (entry == NULL) {
			break;
		}
		if (strcmp(entry->name, pathname) == 0) {
			return entry->inode_num;
		}
	}
	return 0;
}

uint16_t find_entry_in_single_indirect(single_indirect_t* si_blk, char* pathname) {
	block_t* current_blk;
	int i;
	uint16_t found = 0;
	for (i = 0; i < BLK_SIZE/4; ++i) {
		current_blk = si_blk->blocks[i];
		if ((found = find_entry_in_block(current_blk,pathname)) != 0) {
			return found;
		}
	}
	return found;
}

uint16_t find_entry_in_double_indirect(double_indirect_t* di_blk, char* pathname) {
	single_indirect_t* current_si_blk;
	int i;
	uint16_t found = 0;
	for (i = 0; i < BLK_SIZE/4; ++i) {
		current_si_blk = di_blk->blocks[i];
		if ((found = find_entry_in_single_indirect(current_si_blk, pathname)) != 0) {
			return found;
		}
	}
	return found;
}

/* find free inode */
inode_t* get_free_inode(fs_t* fs) {
	inode_t* inode;
	int i;
	for (i = 0; i < NUM_INODES; ++i) {
		inode = &fs->inodes[i];
		if (strcmp(inode->type, "") == 0) {
			/* this inode is not yet used */
			inode->num = i;
			return inode;
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
	strncpy(prefix, pathname, cutoff);
	strcpy(filename, &pathname[cutoff+1]);
}

/* get free entry in dir */
dir_entry_t* get_free_entry(fs_t* fs, inode_t* parent) {
	int i, region = parent->size / BLK_SIZE;
	block_t* current_blk;
	/*dir_entry_t* free_entry;*/
	if (region < 8) {
		/* there is still space in direct blocks */
		for (i = 0; i < NUM_DIRECT_BLK; ++i) {	
			if (parent->direct_blks[i] == NULL) {
				/* prev block is filled, get a new block */
				if ((parent->direct_blks[i] = get_free_block(fs)) == NULL) {
					printk("<1> No more free blocks\n");
					return NULL;
				}
			}
			current_blk = parent->direct_blks[i];
			return get_free_spot_direct_block(current_blk);
		}
	} else if (region < 72) {
		/* free spot in single indirect */
		current_blk = parent->single_indirect->blocks[region - 8];	
		return get_free_spot_direct_block(current_blk);
	} else if (region < 4168) {
		/* free spot in double indirect */
		region = region - 72;
		current_blk = parent->double_indirect->blocks[region/64]->blocks[region%64];
		return get_free_spot_direct_block(current_blk);
	}
	return NULL;
}

dir_entry_t* get_free_entry_direct_block(block_t* block) {
	int i;
	/*dir_entry_t* current_entry;*/
	for (i = 0; i < BLK_SIZE/16; ++i) {
		if (block->entries[i] == NULL) {
			return &block->entries[i];
		}
	}
	return NULL;
}

/* get free block */
block_t* get_free_block(fs_t* fs) {
	int block_index = next_zero_bitmap(fs->bitmap, 4*BLK_SIZE);
	if (block_index < NUM_BLOCKS) {
		fs->superblock.freeblocks--;
		return &fs->blocks[block_index];
	}
	return NULL;
}
