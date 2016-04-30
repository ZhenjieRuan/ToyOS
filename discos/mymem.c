#include "discos.h"
#include <stdio.h>
#include <stdlib.h>

int rd_creat(char *pathname){
  return 0;
} 

int rd_mkdir(char *pathname){
return 0;
} 

int rd_close(int fd){
return 0;
} 

int rd_read(int fd, char *address, int num_bytes){
return 0;
}

int rd_write(int fd, char *address, int num_bytes){
return 0;
}

int rd_lseek(int fd, int offset){
return 0;
}

int rd_unlink(char *pathname){
return 0;
}

int rd_readdir(int fd, char *address){
return 0;
} 

/*
int scan_bitmap(unsigned char *ch, int len){
  int i, bit;
  
  for(i=0; i<len; i++){
    if(ch != 
  }
	
  return -1;
}
*/
int scan_byte(unsigned char ch){
  int i;
  for(i=0; i<8; i++) {
    if((ch & (unsigned char) 1) == 0) {
      return (int)i;
    }
    ch = ch >> 1;
  }

  return -1;
}

void print_byte(unsigned char ch){
  int i;
  for(i=0; i<8; i++) {
    int bit = (ch >> i) & 1;
    printf("%d", bit );

  }
  printf("\n");

}

int main() {
  unsigned char ch = 254;
  printf("print byte\n");
  print_byte(ch);
  printf("first unset bit of char 0 at \n");
  printf("unset bit at %d \n", scan_byte(ch));
  
  //  unsigned char arr[3] = { 255 , 255, 3 };
  
  //printf("first unset char at \n");
  


  return 0;
}


