#include "fifos.h"

/*==============================================*/
/*================== FIFOS =====================*/
/*==============================================*/

static int counter = 0;

uint32_t stack1[STACK_SIZE];
uint32_t stack2[STACK_SIZE];
uint32_t stack3[STACK_SIZE];


static TCB * head;
static TCB * tail;
static TCB * curr_tcb; // the thread that's currently running

static int limit = 0;

/*===================thread====================*/
void interrupt_handler(void) {
	/*terminal_writestring("Got interrupt\n");*/
	send_eoi();
	thread_schedule();
	/*terminal_writestring("Here");*/
}

void yield(void) {
	thread_schedule();
}

void thread1(void) {
	/*STI;*/
	int i = 0;
	int wait = 1000000;
	for (i = 0; i < 10; ++i) {
		threadPrint("1");
		while (wait--);
		wait = 1000000;
		/*yield();*/
	}
	threadDone("1");
	curr_tcb->status = IDLE;
}

void thread2(void) {
	/*STI;*/
	int i = 0;
	int wait = 1000000;
	for (i = 0; i < 10; ++i) {
		threadPrint("2");
		while (wait--);
		wait = 1000000;
		/*yield();*/
	}
	threadDone("2");
	curr_tcb->status = IDLE;
}


void thread3(void) {
	/*STI;*/
	int i = 0;
	int wait = 1000000;
	for (i = 0; i < 10; ++i) {
		threadPrint("3");
		while (wait--);
		wait = 1000000;
		/*yield();*/
	}
	threadDone("3");
	curr_tcb->status = IDLE;
}

int get_tid(void) {
	if (counter == MAX_THREADS)
		return -1;
	return counter++;
}

void thread_schedule(void) {
	CLI;
	if (allDone()) {
		while(1){}
	}
	TCB * src_tcb;
	TCB * des_tcb;

	des_tcb = runqueue_pop();
	if (curr_tcb && curr_tcb->status == BUSY) {
		runqueue_add(curr_tcb);
	}
	if (!des_tcb) return; // no more thread in queue to run

	src_tcb = curr_tcb;

	curr_tcb = des_tcb;

	/*if (src_tcb == 0) {*/
		/*terminal_writestring("Entering thread 1 sp:");*/
		/*terminal_writehex(*((uint32_t *)des_tcb));*/
	/*}*/

	/*while(1) {}*/
	/*terminal_writestring("In schedule\n");*/

	/*if (src_tcb != 0) {*/
		/*terminal_writestring("Switching from ");*/
		/*terminal_writeint(src_tcb->tid);*/
		/*terminal_writestring(" to ");*/
		/*terminal_writeint(des_tcb->tid);*/
		/*terminal_writestring(" ");*/
	/*}*/
	__asm__ volatile ("call switch_to"::"S" (src_tcb), "D" (des_tcb));
}

int thread_create(void *stack, void *func) {
	int new_tid = -1;
	new_tid = get_tid();

	*(((uint32_t *)stack) - 0) = (uint32_t) thread_schedule;
	/**(((uint32_t *)stack) - 0) = (uint32_t) thread_exit;*/
	stack = (void *)(((uint32_t *)stack) - 1);
	thread_pool[new_tid].tid = new_tid;
	thread_pool[new_tid].sp = (uint32_t)(((uint32_t *)stack) - 9);
	thread_pool[new_tid].bp = (uint32_t)stack;
	thread_pool[new_tid].func = (uint32_t)func;
	thread_pool[new_tid].status = BUSY;
	thread_pool[new_tid].next = 0;

	/* setting up stack */
	*(((uint32_t *)stack) - 0) = (uint32_t)func;
	*(((uint32_t *)stack) - 1) = 0; /* EFLAGS */
	*(((uint32_t *)stack) - 2) = 0; /* EAX */
	*(((uint32_t *)stack) - 3) = 0; /* ECX */
	*(((uint32_t *)stack) - 4) = 0; /* EDX */
	*(((uint32_t *)stack) - 5) = 0; /* EBX */
	*(((uint32_t *)stack) - 6) = 0; /* ESP */
	*(((uint32_t *)stack) - 7) = 0; /* EBP */
	*(((uint32_t *)stack) - 8) = 0; /* ESI */
	*(((uint32_t *)stack) - 9) = 0; /* EDI */

	return new_tid;
}

void thread_init(void) {
	int tid = 0;
	tid = thread_create((void*)&stack1[STACK_SIZE - 1], thread1);
	runqueue_add(&thread_pool[tid]);
	tid = thread_create((void*)&stack2[STACK_SIZE - 1], thread2);
	runqueue_add(&thread_pool[tid]);
	tid = thread_create((void*)&stack3[STACK_SIZE - 1], thread3);
	runqueue_add(&thread_pool[tid]);
}

void thread_exit(void) {
	terminal_writestring("Done");
	thread_schedule();
}

int allDone(void) {
	int i = 0;
	for (i = 0; i < MAX_THREADS; ++i) {
		if (thread_pool[i].status == BUSY)
			return 0;
	}
	return 1;
}
/*===================runqueue====================*/
void runqueue_add(TCB* thread) {
	int i = 0;
	if (!head) {
		head = thread;
		tail = thread;
	} else {
		tail->next = thread;
		tail = tail->next;
	}
}

TCB* runqueue_pop(void) {
	TCB* ret;
	if (head) {
		ret = head;
		head = head->next;
		ret->next = 0;
		return ret;
	} else {
		return 0;
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

  terminal_writestring("FIFOS: Welcome *** System memory is: ");
  terminal_writestring(memstr);
  terminal_writestring("MB\n");
	/* enable interrupt */
	init_pic();
	init_pit();
	thread_init();
	STI;
	/*thread_schedule();*/

	while(1){}
}

