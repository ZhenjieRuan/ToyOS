/* 
   Test Bitmap memory interface. Lets you treat the bitmap as 
   a bit addressable data structure. 
*/
#include "bitmap_interface.h"

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


