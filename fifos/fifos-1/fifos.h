#ifndef _FIFOS_H_
#define _FIFOS_H_



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
#define STACK_SIZE  4096
#define IDLE 0
#define BUSY 1
#define FINISHED 2

#define PIC1_BASE_IRQ 0x20
#define PIT_FREQ 1193181        /* in Hz */
#define HZ 500
#define STI __asm__ volatile ("sti")
#define CLI __asm__ volatile ("cli")

/* printing functions*/
void itoa(char*,int,int);
void terminal_initialize(void);
void terminal_writestring(const char*);
void terminal_putchar(char);
void terminal_writehex(uint32_t);
void terminal_writeint(uint32_t);

/* fifos */

/* thread control block */
typedef struct tcb {
	uint32_t sp;
	uint32_t bp;
	uint32_t func;
	int status;
	int tid;
	struct tcb *next;
}__attribute__((packed)) TCB;

struct tcb runqueue[MAX_THREADS];
struct tcb thread_pool[MAX_THREADS];

/* runqueue */
void runqueue_add(TCB*);
TCB* runqueue_pop(void); /* remove tcb with specific tid */

/* thread functions */
void yield(void);
void thread_init(void);
void thread_schedule(void);
void thread_exit(void);
int thread_create(void *stack, void *func);
int get_tid(void);

/* interrupt handling */
void init_pic(void);
void init_pit(void);
void send_eoi(void);

#endif
