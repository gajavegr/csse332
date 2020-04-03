# team member: Ganesh Gajavelli
all: floppystuff kernelstuff writemessage

floppystuff: bootload zerofloppya copybootload

bootload: bootload.asm
		nasm bootload.asm 

zerofloppya: kernel.c
		dd if=/dev/zero of=floppya.img bs=512 count=2880

copybootload: kernel.c
		dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc

kernelstuff: kernel
		dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3

kernel.o: kernel.c
		bcc -ansi -c -o kernel.o kernel.c

kernel_asm.o: kernel.asm
		as86 kernel.asm -o kernel_asm.o

kernel: kernel.o kernel_asm.o
		ld86 -o kernel -d kernel.o kernel_asm.o

writemessage: message.txt kernel
		dd if=message.txt of=floppya.img bs=512 count=1 seek=30 conv=notrunc

clean:
		rm -f kernelstuff floppystuff bootload floppya.img kernel writemessage *.o

