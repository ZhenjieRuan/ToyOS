#include "discos.h"
#include <stdio.h>

int next_zero_byte(unsigned char ch){
  //Find index of next unset bit in byte.
  int i;
  for(i=0; i<8; i++) {
    if((ch & (unsigned char) 1) == 0) {
      return (int)i;
    }
    ch = ch >> 1;
  }
  return -1;
}

int next_zero_bitmap(unsigned char *ch, int len){
  //Find index of next unset bit in bitmap.
  int i, bit;
  bit = 0;
  for(i=0; i<len; i++){
    printf("ch[%d] = %u \n", i, ch[i]);
    if(ch[i] == 255){
      bit += 8;
    }else {
      return bit + next_zero_byte(ch[i]);
    }
  }
  return -1;
}

void print_byte(unsigned char ch){
  //Printing 
  int i;
  for(i=0; i<8; i++) {
    int bit = (ch << i) & 1;
    printf("%d", bit );
  }
  printf("\n");
}

unsigned char set_bit_of_byte(unsigned char *ch, int len){
  *ch = *ch | (1 << len);
  return *ch;
}

int set_bit_of_bitmap(unsigned char *ch, int len){
  int byte = len / 8;
  int bit = len % 8;
  ch[byte] = set_bit_of_byte(&ch[byte], bit);
  
  return 1;
}


int main() {
  unsigned char ch = 1;
  print_byte(ch);

  /*  
unsigned char arr[3] = { 0 , 0, 0 };
 
  set_bit_at(arr, 5);
  unset_bit_at(arr, );
  
  int i;
  for(i=0; i<3; i++){
    printf("arr[%d] = %u \n", i, arr[i]);
  }
*/
  return 0;
}


