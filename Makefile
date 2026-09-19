CC = x86_64-elf-gcc
LD = x86_64-elf-ld

#GRUB_FILE = i686-elf-grub-file
#GRUB_MKRESCUE = i686-elf-grub-mkrescue
GRUB_FILE = x86_64-elf-grub-file
GRUB_MKRESCUE = x86_64-elf-grub-mkrescue

CFLAGS = \
	-m32 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pie \
	-mno-red-zone \
	-fno-asynchronous-unwind-tables \
	-fno-unwind-tables

LDFLAGS = \
	-m elf_i386 \
	-T linker.ld \
	-o kernel


all: MihateOS.iso


boot.o: boot.S
	$(CC) -m32 -c boot.S -o boot.o


kernel.o: kernel.c graphics.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o


graphics.o: graphics.c graphics.h
	$(CC) $(CFLAGS) -c graphics.c -o graphics.o

gdt.o: gdt.c gdt.h
	$(CC) $(CFLAGS) -c gdt.c -o gdt.o

gdt_flush.o: gdt_flush.S
	$(CC) -m32 -c gdt_flush.S -o gdt_flush.o

idt.o: idt.c idt.h
	$(CC) $(CFLAGS) -c idt.c -o idt.o

pic.o: pic.c pic.h io.h
	$(CC) $(CFLAGS) -c pic.c -o pic.o

keyboard.o: keyboard.c keyboard.h io.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

interrupt.o: interrupt.S
	$(CC) -m32 -c interrupt.S -o interrupt.o

kernel: boot.o \
        gdt.o \
        gdt_flush.o \
        idt.o \
        pic.o \
        keyboard.o \
        interrupt.o \
        kernel.o \
        graphics.o \
        linker.ld

	$(LD) $(LDFLAGS) \
        boot.o \
        gdt.o \
        gdt_flush.o \
        idt.o \
        pic.o \
        keyboard.o \
        interrupt.o \
        kernel.o \
        graphics.o


check: kernel
	$(GRUB_FILE) --is-x86-multiboot2 kernel


iso/boot/kernel: kernel
	mkdir -p iso/boot/grub
	cp kernel iso/boot/kernel


iso/boot/grub/grub.cfg: grub.cfg
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/grub.cfg


MihateOS.iso: iso/boot/kernel iso/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o MihateOS.iso iso


run: MihateOS.iso
	qemu-system-x86_64 \
    	-machine q35 \
    	-m 512M \
    	-drive if=pflash,format=raw,readonly=on,file=./edk2-stable202605-r1-bin/x64/code.fd \
    	-drive if=pflash,format=raw,file=./edk2-stable202605-r1-bin/x64/vars.fd \
    	-cdrom MihateOS.iso \
    	-display cocoa,zoom-to-fit=on \
		-debugcon stdio \
    	-global isa-debugcon.iobase=0xe9 \
    	-no-reboot


clean:
	rm -rf *.o kernel MihateOS.iso iso