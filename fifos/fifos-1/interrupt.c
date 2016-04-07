#include "fifos.h"

int counter = 0;

static inline void
outb (uint8 uch, uint16 usPort) {
  __asm__ volatile ("outb %0,%1"::"a" (uch), "Nd" (usPort));
}

void send_eoi (void) {
  outb (0x20, 0x20);          /* send to 8259A PIC */
}


void init_pic(void) {
  /* Master PIC */
  outb (0x11, 0x20);            /* 8259 (ICW1) - xxx10x01 */
  outb (PIC1_BASE_IRQ, 0x21);   /* 8259 (ICW2) - set IRQ0... to int 0x20... */
  outb (0x0D, 0x21);            /* 8259 (ICW4) - Buffered master, normal EOI, 8086 mode */
}

/* Programmable interval timer settings */
void init_pit (void)
{

  outb (0x34, 0x43);            /* 8254 (control word) - counter 0, mode 2 */

  /* Set interval timer to interrupt once every 1/HZth second */
  outb ((PIT_FREQ / HZ) & 0xFF, 0x40);  /* counter 0 low byte */
  outb ((PIT_FREQ / HZ) >> 8, 0x40);    /* counter 0 high byte */
}

/*void timer_handler(void) {*/
	/*if (counter == 500) {*/
		/*terminal_writestring("In Interrupt handler\n");*/
		/*counter = 0;*/
	/*} else {*/
		/*counter++;*/
	/*}*/
	/*send_eoi();*/
/*}*/
