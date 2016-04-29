//Reference for mem layout. 

#include <stdio.h>

struct block{
  char data[256];
};

struct inode{ 
  struct block *direct[8];
  struct block *indirect;
  struct block *double_indirect;
  //Allocate this properly. 
  char other[24];
};

struct fs{
  //256
  struct block superblock;
  //256^2 = 64*1024
  struct inode inodes[1024];
  //256*4 
  char bitmap[1024];
  //2^21 -66816 =  2^13 -261
  struct block blocks[7931];
};

int main(){
  struct block b;
  struct inode c;
  struct fs filesys;

  printf("size block %lu\n", sizeof(b));
  printf("size inode %lu\n", sizeof(c));
  //256 + 256^2 + 4 * 256 + data
  printf("sizeof fs = %lu \n", sizeof(filesys));
  return 0;
}

