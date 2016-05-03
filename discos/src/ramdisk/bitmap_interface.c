/* 
   Bitmap memory interface. Lets you treat the bitmap as 
   a bit addressable data structure. 
*/

#include "discos_kernel.h"
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

int main() {
  unsigned char arr[3] = { 0, 0, 0 };
  
  printf("Starting with all zeros\n");
  print_arr( arr, 3);
  printf("First free bit at %d \n", next_zero_bitmap(arr, 3));

  printf("set 0th bit \n");
  set_bit_of_bitmap(arr, 0);
  print_arr( arr, 3);
  printf("Next free bit at %d \n", next_zero_bitmap(arr, 3));  
  
  printf("set first byte\n");
  arr[0] = 255;
  print_arr( arr, 3);
  printf("Next free bit at %d \n", next_zero_bitmap(arr, 3));  

  printf("unset bit 3 of first byte\n");
  unset_bit_of_bitmap( arr, 3);
  printf("Next free bit at %d \n", next_zero_bitmap(arr, 3));    
  print_byte(arr[0]);

  printf("new array  \{ 255 , 255, 5 } \n");
  unsigned char myarr[3] = { 255, 255, 5 };
  print_arr( arr, 3);
  printf("Next free bit at %d \n", next_zero_bitmap(myarr, 3));    

  set_next_free_bit(myarr);
  printf("Next free bit at %d \n", next_zero_bitmap(myarr, 3));    
  set_next_free_bit(myarr);
  printf("Next free bit at %d \n", next_zero_bitmap(myarr, 3));    
  set_next_free_bit(myarr);
  printf("Next free bit at %d \n", next_zero_bitmap(myarr, 3));    

  

  return 0;
}


