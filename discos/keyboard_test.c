/* 
 * Keyboard Module Demonstration Program
 */
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_KEYBOARD _IOR(0, 6, struct keyboard_char_t)

/** The struct passed between the module and the program */
struct keyboard_char_t {
  char character;
};

static struct keyboard_char_t keyboard_char;

/** File descriptor to kernel module */
static int fd;

/** Wrap getchar() with our own kernel module solution */
char __wrap_getchar(void) {
    int response;
    response = ioctl (fd, IOCTL_KEYBOARD, &keyboard_char);
    if (response == -1 ) {
	perror("OOPS! Something went wrong... Did you enable the module?\n");
        exit(1);
    }
    return keyboard_char.character;
}
/** Use a buffer to display what the kernel gives us. */
int main () {
  char buffer[250];
  char *out_of_bounds = &buffer[250];
  char *rp, *wp;
  wp = rp = buffer;
  char c;
  fd = open ("/proc/ioctl_keyboard_test", O_RDONLY);
  printf("******************************\n"
	 "Welcome to the Basic Keyboard Driver! "
	 "Type in a line and see how the keyboard "
	 "driver interpreted it.\n"
	 "You can use shift keys, but they only "
	 "work properly on letters.\n");

  while(1) {
    while((c = getchar()) != '\n' ) {
      *wp++ = c;
      if(wp == out_of_bounds) {
      	wp = buffer;
      }
    }
    printf("\nThe kernel module interpreted:\n");
    for(rp; rp != wp; rp++) {
        if(rp == out_of_bounds) {
          rp = buffer;
        }
	putchar(*rp);
    }
    putchar('\n');
  }
  return 0;
}
