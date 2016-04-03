#include "multiboot.h"
#include "types.h"

 /*ini*/
/*init_pcb();*/

 /*initialize 8254 PIC*/
/*init_pic();*/

/*init 8254 pit, use mode 2*/
/*init_pit();*/

#define crBegin static int state=0; switch(state) { case 0:

/*
 * ANSI C provides the __LINE__ macro to return the current source line number
 * We can use the line number as a simple form of programmatic state!
 */
#define crReturn(x) do { state=__LINE__; return; \
                         case __LINE__:; } while (0)
#define crFinish }

#define threadPrint(x) terminal_writestring("<"); \
											 terminal_writestring(x); \
											 terminal_writestring(">");

#define threadDone(x) terminal_writestring("Done <"); \
											terminal_writestring(x); \
											terminal_writestring(">\n");

#define MAX_THREADS 3


typedef struct tcb {
	void (*entry)();
	int tid;
	void *sp;
	void *bp;
} tcb;

typedef struct thread {
	tcb info;
	int state;
} thread;

static thread threads[MAX_THREADS];
static int current_tid;
static bool done[MAX_THREADS];

void (*f[MAX_THREADS])(void);

/* Hardware text mode color constants. */
enum vga_color
{
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};
 
uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
  return fg | bg << 4;
}
 
uint16_t make_vgaentry(char c, uint8_t color)
{
  uint16_t c16 = c;
  uint16_t color16 = color;
  return c16 | color16 << 8;
}
 
size_t strlen(const char* str)
{
  size_t ret = 0;
  while ( str[ret] != 0 )
    ret++;
  return ret;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 24;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize()
{
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
  terminal_buffer = (uint16_t*) 0xB8000;
  for ( size_t y = 0; y < VGA_HEIGHT; y++ )
    {
      for ( size_t x = 0; x < VGA_WIDTH; x++ )
	{
	  const size_t index = y * VGA_WIDTH + x;
	  terminal_buffer[index] = make_vgaentry(' ', terminal_color);
	}
    }
}
 
void terminal_setcolor(uint8_t color)
{
  terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = make_vgaentry(c, color);
}
 
void terminal_putchar(char c)
{
  terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
  if ( ++terminal_column == VGA_WIDTH )
    {
      terminal_column = 0;
      if ( ++terminal_row == VGA_HEIGHT )
	{
	  terminal_row = 0;
	}
    }
}
 
void terminal_writestring(const char* data)
{
  size_t datalen = strlen(data);
  for ( size_t i = 0; i < datalen; i++ )
    terminal_putchar(data[i]);
}
 

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal. */
void itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
     
  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
     
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);
     
  /* Terminate BUF. */
  *p = 0;
     
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

void thread1 (void) {
	int i;
	
	for (i = 0; i < 10; ++i) {
		terminal_writestring("<1>");
	}

	done[0] = TRUE;

	terminal_writestring("done <1>");
}

void thread2 (void) {
	int i;

	for (i = 0; i < 10; ++i) {
		terminal_writestring("<2>");
	}

	done[1] = TRUE;
	
	terminal_writestring("done <2>");
}

void thread3 (void) {
	int i;
	
	for (i = 0; i < 10; ++i) {
		terminal_writestring("<3>");
	}


	done[2] = TRUE;
	
	terminal_writestring("done <3>");
}

void init_tcb() {
    f[0] = thread1;
    f[1] = thread2;
    f[2] = thread3;

    current_tid = 0;

    int i;
    for (i = 0; i < MAX_THREADS; i++) {
        threads[i].info.tid = -1;
        threads[i].state = 0;
        done[i] = FALSE;
    }

}

int thread_create(void *stack, void (*func) ()) {    
    int tid = current_tid + 1;

    threads[current_tid].info.tid = tid;
    threads[current_tid].info.entry = func;

    // Set stack pointer
    threads[current_tid].info.sp = stack;

    current_tid = (current_tid + 1) %  MAX_THREADS;
    return tid;
}

void init_threads() {
    int i;
		void* stack[3];
		uint32_t stack1[1024];
		uint32_t stack2[1024];
		uint32_t stack3[1024];
		stack[0] = &stack1;
		stack[1] = &stack2;
		stack[2] = &stack3;
    for (i = 0; i < MAX_THREADS; i++) {
        thread_create(stack[i], f[i]);
    }
}

int Done() {
	int i;
	for (i = 0; i < MAX_THREADS; ++i) {
		if (done[i] == FALSE) return 0; 
	}
	return 1;
}

void schedule(void) {
  int current = current_tid;
  int finished;
  int thread_count = 0;
  
  int i;
  for (i = 0; i < MAX_THREADS; i++) {
    threads[i].state = 1;
  }

  while (threads[current].state == 1 || Done() == 0) {
    if (threads[current].state == 1)
        (threads[current].info.entry) ();

    if (done[threads[current].info.tid - 1] && threads[current].state == 1) {
        if (thread_count == MAX_THREADS - 1) {
            return;
        }

        finished = current;
        threads[current].state = 0;
        thread_count++;
    }
    current = (current + 1) %  MAX_THREADS;
  }
}



void init( multiboot* pmb ) {

   memory_map_t *mmap;
   unsigned int memsz = 0;		/* Memory size in MB */
   static char memstr[10];

  for (mmap = (memory_map_t *) pmb->mmap_addr;
       (unsigned long) mmap < pmb->mmap_addr + pmb->mmap_length;
       mmap = (memory_map_t *) ((unsigned long) mmap
				+ mmap->size + 4 /*sizeof (mmap->size)*/)) {
    
    if (mmap->type == 1)	/* Available RAM -- see 'info multiboot' */
      memsz += mmap->length_low;
  }

  /* Convert memsz to MBs */
  memsz = (memsz >> 20) + 1;	/* The + 1 accounts for rounding
				   errors to the nearest MB that are
				   in the machine, because some of the
				   memory is othrwise allocated to
				   multiboot data structures, the
				   kernel image, or is reserved (e.g.,
				   for the BIOS). This guarantees we
				   see the same memory output as
				   specified to QEMU.
				    */

  itoa(memstr, 'd', memsz);

  terminal_initialize();

  terminal_writestring("MemOS: Welcome *** System memory is: ");
  terminal_writestring(memstr);
  terminal_writestring("MB");
	
	init_tcb();
	init_threads();
	schedule();
}
