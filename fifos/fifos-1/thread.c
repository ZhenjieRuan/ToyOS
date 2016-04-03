#include "types.h"


typedef struct _PCB {
	uint32_t sp; // stack pointer 
	uint32_t bp; // base pointer
	uint32_t entry; // entry point (func)
	uint32_t tid; // thread id
	uint32_t flag; // no use here
	struct PCB * next; // pointer to next process control block
} __attribute__((packed)) PCB;


void init_pcb(void) {
	int i = 0;
	/* clear global PCB pool */

}
