vga16:
	as vga16.s -o vga16.o
	ld -T vga.ld vga16.o -o vga16
	dd bs=1 if=vga16 of=vga16_test skip=4096 count=512

memos1:
	as memos-1.s -o memos-1.o
	ld -T memos.ld memos-1.o -o memos1
	dd bs=1 if=memos1 of=memos1_test skip=4096 count=512

testmemos1:
	qemu-system-i386 -hda memos1_test

testvga:
	qemu-system-i386 -hda vga16_test

clean:
	rm vga16.o vga16 vga16_test memos-1.o memos1 memos1_test
