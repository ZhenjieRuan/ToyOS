## Preemption (8253/54 PIT)

+ Timer interrupt
+ Need to setup a basic flat memory model
  + 0-4GB for code & data
  + can assume the segment for code & data are set with kernel privileges
  + about (a) The segment descriptor in the GDT which contains the interrupt handler
  + about (b) the offset address in segment for the start address of the handler
  + Read dev manual Vol 3 Ch.6.2 Ch.3
+ Set up GDT(Global Descriptor Table)
  + Set up in memory
  + lgdt - load global descriptor table to register
  + lidt - load interrupt descriptor table to register

| 1 NULL <- 8-byte descriptor entries|
| :------------- |
| 2 Kernel CODE |
| 3 Kernel DATA |

## Resource
+ osdev.org/PIC

## Thread Creation

thread_create(void \*stack, void \*func)

| thread stack     |
| :------------- |
| return routine (4-bytes)       |
| EIP -> return address from where I was called|
| FLAGS -> machine flags register|
| EAX |
| ... machine registers, saved by a pusha instruction|
| ... |
| DS -> segment register|
| ES -> segment register|

two memory manipulation: timer interrupt, context switch
