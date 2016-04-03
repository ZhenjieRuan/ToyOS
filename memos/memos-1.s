#memos 1
#Author: Zhenjie Ruan
#simple os that display avaliable memory when boot up

#generate 16-bit code
.code16

#hint the assembler that here is the executable code located
.globl _start;
#boot code entry
_start:
      jmp _boot                           #jump to boot code
      welcome: .asciz "Hello, World\n\r"  #here we define the string

     .macro mWriteString str              #macro which calls a function to print a string
          leaw  \str, %si
          call .writeStringIn
     .endm

     #function to print the string
     .writeStringIn:
          lodsb
          orb  %al, %al
          jz   .writeStringOut
          movb $0x0e, %ah
          int  $0x10
          jmp  .writeStringIn
     .writeStringOut:
     ret

_boot:
     mWriteString welcome

		 .org 0x1FE

     .byte 0x55
     .byte 0xaa  
