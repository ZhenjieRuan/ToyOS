			.global stack
			.global idt

			.bss
			.align 0x1000
			.comm stack, 0x1000
			.comm idt /* interrupt descriptor table */, 0x800

			.data
			.align 0x4
      
 gdt:/* global descriptor table (find more in intel dev manual vol 3) */
			/* null decriptor -- 1st (Zeroth) entry not used used to catch seg fault */
			.long 0
			.long 0

			/* kernel CS */
			/* data decriptor - base 0, limit 0xFFFFFFFF, 32-bit, readable, ring 0 */
			.long 0x0000FFFF
			.long 0x00CF9A00

			/* Kernel DS */
			/* data desciptor - base 0, limit 0xFFFFFFFF, 32-bit, writable, ring 0 */
			.long 0x0000FFFF
			.long 0x00CF9200

gdt_ptr:
			.short 0x18 /* only need space for 3 descriptors, don't need 256 only need 24 */
			.long gdt

idt_ptr:
			.short 0x7FF
			.long idt

			.text

			.global _start

_start:
			jmp real_start

			/* Multiboot header -- Safe to place this header in 1st page for GRUB */
			.align 4
			.long 0x1BADB002 /* Multiboot magic number */
			.long 0x00000003 /* Align modules to 4KB, req. mem size */
					 /* See 'info multiboot' for further info */
			.long 0xE4524FFB /* Checksum */

real_start:
			/* switch to our own GDT */
			lgdt gdt_ptr
			ljmp $0x08, $1f
1:
			movw $0x10, %ax /* used to setup second descriptor selector for gdt */
			movw %ax, %ss /* 2nd decriptor in GDT references kernel data segment */
			movw %ax, %ds
			movw %ax, %es
			movw %ax, %fs
			movw %ax, %gs

			/* set up stack */
			movl $stack+0x1000, %esp /* setup 4Kb stack */

			/* save multiboot parameter, for evetual call to C code */
			pushl %ebx

			/* set up IDT by construction 64-bit iterrupt desciptors 
			 Look at Ch.6 in the manual */ 
			movl $idt, %edi
			movl $0x30, %ecx  /* set counter for 48 IDT entries used */
			movl $0x080000, %edx /* Used to index kernel CS */
			movl $int_table, %esi

			/*
			 * For now populate int_table with interrupt gate descriptors
			 * These clear the IF flag when invoked
			 */
1:
			lodsl // value at address DS:ESI --> EAX
			movw %ax, %dx // lower 16-bits  of int_table address (See: Fig 6.2)
			movw $0xEE00, %ax // DPL = (ring) 3
			movl %edx, (%edi) // store bottom 32-bits of descriptor in EDX to IDT 
			addl $4, %edi
			stosl // store top 32-bits of descriptor held in EAX into IDT EAX --> ES:EDI

			loop 1b
			
			lidt idt_ptr
			
			call init // call c code

			cli
			hlt

loop:
			jmp loop

int_table:
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long timer

timer:
		call timer_handler
