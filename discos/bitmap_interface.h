/* 
   Bitmap memory interface. Lets you treat the bitmap as 
   a bit addressable data structure. 
*/

#include "discos.h"
#include <stdio.h>

int next_zero_bit(unsigned char ch){
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

int next_zero_bitmap(unsigned char *map, int len){
  //Find index of next unset bit in bitmap.
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

void print_byte(unsigned char ch){
  //Printing byte. LSB on the right. 
  int i;
  for(i=0; i<8; i++) {
    int bit = (ch & (1 << (7 - i))) != 0;
    printf("%d", bit );
  }
  printf("\n");
}

void set_bit_of_byte(unsigned char *ch, int len){
  //Set bit at offset len.
  *ch = *ch | (1 << len);
}

void set_bit_of_bitmap(unsigned char *map, int len) {
  //Assert bit at offset len.
  int byte = len / 8;
  int bit = len % 8;
  set_bit_of_byte(&map[byte], bit);
}

void unset_bit_of_byte(unsigned char *ch, unsigned char len) {
  //Deassert bit at offset len.
  *ch = *ch & ~(1 << len);
}

void unset_bit_of_bitmap(unsigned char *map, int len) {
  //Deassert bit at offset len.
  int byte = len / 8;
  int bit = len % 8;
  unset_bit_of_byte(&map[byte], bit);
}

void print_arr(unsigned char *arr, int len) {
  //Print unsigned char rep of arr.
  int i;
  for(i=0; i<3; i++) {
    printf("arr[%d] = %u \n",i, arr[i] );
  }
}

void set_next_free_bit(unsigned char *map) {
  //Some error checking would be nice. 
  set_bit_of_bitmap(map, next_zero_bitmap( map, 3));  
}

