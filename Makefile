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


all: myos.iso


boot.o: boot.S
	$(CC) -m32 -c boot.S -o boot.o


kernel.o: kernel.c graphics.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o


graphics.o: graphics.c graphics.h
	$(CC) $(CFLAGS) -c graphics.c -o graphics.o


kernel: boot.o kernel.o graphics.o linker.ld
	$(LD) $(LDFLAGS) boot.o kernel.o graphics.o


check: kernel
	$(GRUB_FILE) --is-x86-multiboot2 kernel


iso/boot/kernel: kernel
	mkdir -p iso/boot/grub
	cp kernel iso/boot/kernel


iso/boot/grub/grub.cfg: grub.cfg
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/grub.cfg


myos.iso: iso/boot/kernel iso/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o myos.iso iso


run: myos.iso
	qemu-system-x86_64 \
    	-machine q35 \
    	-m 512M \
    	-drive if=pflash,format=raw,readonly=on,file=./edk2-stable202605-r1-bin/x64/code.fd \
    	-drive if=pflash,format=raw,file=./edk2-stable202605-r1-bin/x64/vars.fd \
    	-cdrom myos.iso \
    	-display cocoa,zoom-to-fit=on \
		-debugcon stdio \
    	-global isa-debugcon.iobase=0xe9 \
    	-no-reboot


clean:
	rm -rf *.o kernel myos.iso iso