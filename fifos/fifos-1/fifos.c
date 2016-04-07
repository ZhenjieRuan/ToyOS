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

/*===================thread====================*/
void yield(void) {
}

void thread1(void) {
	int i = 0;
	int j = 0;
	while (1) {
		for (i = 0; i < 5; ++i) {
			threadPrint("1");
		}
		if (++j > 2)
			break;
	}
	thread_pool[0].status = FINISHED;
	thread_exit();
}

void thread2(void) {
	int i = 0;
	int j = 0;
	while (1) {
		for (i = 0; i < 5; ++i) {
			threadPrint("2");
		}
		if (++j > 2)
			break;
	}
	thread_pool[1].status = FINISHED;
	thread_exit();
}


void thread3(void) {
	int i = 0;
	int j = 0;
	while (1) {
		for (i = 0; i < 5; ++i) {
			threadPrint("3");
		}
		if (++j > 2)
			break;
	}
	thread_pool[2].status = FINISHED;
	thread_exit();
}

int get_tid(void) {
	if (counter == MAX_THREADS)
		return -1;
	return counter++;
}

/*void *memset(void *s, int c, size_t n) {*/
	/*unsigned char *p = s;*/
	/*while (n--) {*/
		/**p++ = (unsigned char) c;*/
	/*}*/
	/*return s;*/
/*}*/

int thread_create(void *stack, void *func) {
	int new_tid = -1;
	/*uint16_t ds=0x10,es=0x10,fs=0x10,gs=0x10;*/
	new_tid = get_tid();

	*(((uint32_t *)stack) - 0) = (uint32_t) thread_exit;
	stack = (void *)(((uint32_t *)stack) - 1);
	thread_pool[new_tid].tid = new_tid;
	thread_pool[new_tid].sp = (uint32_t)(((uint32_t *)stack) - 9);
	thread_pool[new_tid].bp = (uint32_t)stack;
	thread_pool[new_tid].func = (uint32_t)func;
	thread_pool[new_tid].status = IDLE;
	thread_pool[new_tid].next = 0;

	/* setting up stack */
	*(((uint32_t *)stack) - 0) = (uint32_t)func;
	*(((uint32_t *)stack) - 1) = 2; /* EFLAGS */
	*(((uint32_t *)stack) - 2) = 0; /* EAX */
	*(((uint32_t *)stack) - 3) = 0; /* ECX */
	*(((uint32_t *)stack) - 4) = 0; /* EDX */
	*(((uint32_t *)stack) - 5) = 0; /* EBX */
	*(((uint32_t *)stack) - 6) = 0; /* ESP */
	*(((uint32_t *)stack) - 7) = 0; /* EBP */
	*(((uint32_t *)stack) - 8) = 0; /* ESI */
	*(((uint32_t *)stack) - 9) = 0; /* EDI */
	/**(((uint16_t *)stack) - 19) = (uint16_t)ds;*/
	/**(((uint16_t *)stack) - 20) = (uint16_t)es;*/
	/**(((uint16_t *)stack) - 21) = (uint16_t)fs;*/
	/**(((uint16_t *)stack) - 22) = (uint16_t)gs;*/

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
		if (thread_pool[i].status != FINISHED)
			return 0;
	}
	return 1;
}

void thread_schedule(void) {
	if (allDone()) {
		return;
	}
	TCB * src_tcb;
	TCB * des_tcb;

	des_tcb = runqueue_pop();
	if (curr_tcb != 0) {
		runqueue_add(curr_tcb);
	}
	if (!des_tcb) return; // no more thread in queue to run

	src_tcb = curr_tcb;

	curr_tcb = des_tcb;

	__asm__ volatile ("call switch_to":
								:"S" (src_tcb), "D" (des_tcb));
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

/*void runqueue_remove(TCB* target) {*/
	/*TCB* curr;*/
	/*TCB* prev;*/
	/*curr = head;*/
	/*while (curr) {*/
		/*if (curr->tid == target->tid) {*/
			/*if (prev) {*/
				/*prev->next = curr->next;*/
				/*curr->next = 0;*/
			/*} else {*/
				/*prev = curr->next;*/
				/*curr->next = 0;*/
				/*curr = prev;*/
			/*}*/
			/*break;*/
		/*}*/
		/*prev = curr;*/
		/*curr = curr->next;*/
	/*}*/
/*}*/



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
	/* enable interrupt */
	/*init_pic();*/
	/*init_pit();*/
	/*STI;*/
	thread_init();
	thread_schedule();

	while(1){}
}

