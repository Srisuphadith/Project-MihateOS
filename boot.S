.section .multiboot, "a"
.align 8

header_start:

    .long 0xE85250D6
    .long 0
    .long header_end - header_start

    # -(0xE85250D6 + 0 + 48)
    .long 0x17ADAEFA

    # Framebuffer tag
    .short 5
    .short 0
    .long 20

    # No specific framebuffer preference
    .long 0
    .long 0
    .long 0

    # Padding
    .long 0

    # End tag
    .short 0
    .short 0
    .long 8

header_end:


.section .text
.global _start
.type _start, @function

.code32

_start:

    cli

    # EAX = Multiboot2 magic
    # EBX = Multiboot2 information address

    movl %ebx, multiboot_info_ptr

    movl $stack_top, %esp

    pushl %ebx
    call kernel_main

halt:
    cli
    hlt
    jmp halt


.section .bss

.align 8

multiboot_info_ptr:
    .long 0

.align 16

stack_bottom:
    .skip 16384

stack_top: