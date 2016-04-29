//Reference for mem layout. 
struct block{
  char data[256];
};

struct inode{ 
  char type;
  int size;
  struct block *direct[8];
  struct block *indirect;
  struct block *double_indirect;
  //Allocate this properly. 
  char other[16];
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

int rd_creat(char *);
int rd_mkdir(char *);
int rd_close(int );
int rd_read(int, char *, int );
int rd_write(int, char *, int);
int rd_lseek(int, int );
int rd_unlink(char *);
int rd_readdir(int, char *);

