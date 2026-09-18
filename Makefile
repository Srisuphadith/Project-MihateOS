CC = x86_64-elf-gcc
LD = x86_64-elf-ld

GRUB_FILE = i686-elf-grub-file
GRUB_MKRESCUE = i686-elf-grub-mkrescue

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

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kernel: boot.o kernel.o linker.ld
	$(LD) $(LDFLAGS) boot.o kernel.o

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
		-m 512M \
		-boot order=d \
		-cdrom myos.iso \
		-display cocoa \
		-no-reboot

clean:
	rm -rf *.o kernel myos.iso iso