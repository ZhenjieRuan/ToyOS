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

/* return parent inode number if valid, -1 on error 
 * Error Condition:
 * 1. Pathname prefix refers to a non-existent directory
 * 2. Try to create existing file
 */
int get_parent_inode(fs_t* fs, char* pathname) {
	int i, found;
	char *token, *string, *tofree;
	block_t* current_blk;
	inode_t* current_inode;
	tofree = string = kstrdup(pathname, GFP_KERNEL);
	if (string != NULL) {
		/* skip the root dir */
		string++;
		/* starting search from the root dir */
		current_inode = fs->inodes[0];

		while ((token = strsep(&string,"/")) != NULL) {
			found = 0;

			/* walk through the direct blocks */
			for (i = 0; i < NUM_DIRECT_BLK; ++i) {
				current_blk = current_inode->direct_blks[i];
				if (current_blk != NULL) {
					if ((found = find_entry_in_block(current_blk, pathname)) != 0) {
						current_inode = fs->inodes[found];	
						break;
					}
				}
			}
			
			/* walk through single-indirect blocks */
			for (i = 0; i < BLK_SIZE/4; ++i) {}
		}	
	}
	return 0;
}

/* search dir block for certain dir, inode number if found, 0 if not found */
uint16_t find_entry_in_block(block_t* block, char* pathname) {
	dir_entry_t* entry;
	int i;
	for (i = 0; i < BLK_SIZE/16; ++i) {
		entry = block->entries[i];
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
