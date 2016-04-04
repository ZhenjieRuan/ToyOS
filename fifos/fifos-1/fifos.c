#include "multiboot.h"
#include "types.h"

 /*ini*/
/*init_pcb();*/

 /*initialize 8254 PIC*/
/*init_pic();*/

/*init 8254 pit, use mode 2*/
/*init_pit();*/

#define threadPrint(x) terminal_writestring("<");\
											 terminal_writestring(x);\
											 terminal_writestring(">");

#define threadDone(x)  terminal_writestring("Done <");\
											 terminal_writestring(x);\
											 terminal_writestring(">\n");

#define MAX_THREADS 3

/*typedef enum {*/
	/*FALSE,*/
	/*TRUE*/
/*} bool;*/


typedef struct tcb {
	int (*func)();
	int tid;
	int busy;
	struct tcb *next;
	struct tcb *prev;
	void* sp;
	void* bp;
	void* stack[1024];
} tcb;

static tcb* pool[MAX_THREADS];
static tcb  *head; /* queue head */
static tcb  *tail; /* queue tail */
static tcb  *current;


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
  for ( size_t i = 0; i < datalen; i++ ) {
		if (data[i] == '\n') {
			terminal_row++;
			terminal_column = 0;
		} else {
			terminal_putchar(data[i]);
		}
	}
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

void thread_func(int tid) {
	char buf[2];
	char *id = &buf[0];
	itoa(id, 10, tid);
	int i = 0;
	for (i = 0; i < 5; ++i) {
		threadPrint(id);
	}
	threadDone(id);
	pool[tid]->busy = 0;
}

void pool_init() {
	int i = 0;
	for (i = 0; i < MAX_THREADS; ++i) {
		pool[i]->tid = i;
	}
}

int thread_create(void *stack, void *func) {
	int i = 0;
	pool[i]->sp = stack - 4;
	pool[i]->busy = 1;
	pool[i]->func = func;
	return pool[i]->tid;
}

int allDone() {
	int i = 0;
	for (i = 0; i < MAX_THREADS; ++i) {
		if (pool[i]->busy != 0) return 0;
	}
	return 1;
}

void thread_init() {
	int i = 0;
	void (*f) (int);
	f = &thread_func;
	for (i = 0; i < MAX_THREADS; ++i) {
		thread_create(&pool[i]->stack[1023], f);
		if (i > 0) {
			pool[i]->prev = pool[i-1];
			pool[i-1]->next = pool[i];
		}
	}
	head = pool[0];
	tail = pool[i];
	current = head;
	terminal_writestring("head thread <");
	terminal_putchar(current->tid + 48);
	terminal_writestring(">\n");
	current->tid = 3;
	current = current->next;
	terminal_writestring("head->next thread <");
	terminal_putchar(current->tid + 48);
	terminal_writestring(">\n");
	current = current->next;
	terminal_writestring("head->next->next thread <");
	terminal_putchar(current->tid + 48);
	terminal_writestring(">\n");
	current = head;
}

void thread_schedule() {
	while (!allDone()) {
		if (current->busy) {
			(current->func)(current->tid);
		} else if (current->next){
			current = current->next;
		}
	}
	terminal_writestring("All Done\n");
}


void timer_handler() {}


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
  terminal_writestring("MB\n");

	pool_init();
	thread_init();
	thread_schedule();
}

