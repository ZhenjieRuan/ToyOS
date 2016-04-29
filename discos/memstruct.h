//Reference for mem layout. 
typedef struct block{
  char data[256];
} block_t;

typedef struct inode{ 
  char type[4];
  int size;
  block_t *direct[8];
  block_t *indirect;
  block_t *double_indirect;
  //Allocate this properly. 
  char other[16];
} inode_t;

typedef struct fs{
  //256
  block_t superblock;
  //256^2 = 64*1024
  inode_t inodes[1024];
  //256*4 
  char bitmap[1024];
  //2^21 -66816 =  2^13 -261
  block_t blocks[7931];
} fs_t;

int rd_creat(char *);
int rd_mkdir(char *);
int rd_close(int );
int rd_read(int, char *, int );
int rd_write(int, char *, int);
int rd_lseek(int, int );
int rd_unlink(char *);
int rd_readdir(int, char *);

