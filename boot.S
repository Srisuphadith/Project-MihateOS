.section .multiboot, "a"
.align 8

header_start:
    .long 0xE85250D6
    .long 0
    .long header_end - header_start
    .long -(0xE85250D6 + 0 + (header_end - header_start))
header_end:


.section .text
.code32

.global _start
.type _start, @function

_start:
    cli

    # ------------------------------------------------
    # GRUB enters us in 32-bit protected mode.
    # EBX contains the Multiboot2 information address.
    # Save it for later.
    # ------------------------------------------------

    mov %ebx, multiboot_info


    # ------------------------------------------------
    # Load our 64-bit GDT
    # ------------------------------------------------

    lgdt gdt64_descriptor


    # ------------------------------------------------
    # Set up page tables
    # ------------------------------------------------

    # PML4[0] -> PDPT
    mov $page_table_l3, %eax
    or $0x03, %eax
    mov %eax, page_table_l4

    # PDPT[0] -> Page Directory
    mov $page_table_l2, %eax
    or $0x03, %eax
    mov %eax, page_table_l3

    # ------------------------------------------------
    # Create identity mapping:
    #
    # 0x00000000 - 0x3FFFFFFF
    #
    # using 2 MiB pages
    # ------------------------------------------------

    mov $0, %ecx

.set_page_table:
    mov %ecx, %eax
    shl $21, %eax

    or $0x83, %eax

    mov %eax, page_table_l2(,%ecx,8)

    inc %ecx
    cmp $512, %ecx
    jne .set_page_table


    # ------------------------------------------------
    # Load PML4 into CR3
    # ------------------------------------------------

    mov $page_table_l4, %eax
    mov %eax, %cr3


    # ------------------------------------------------
    # Enable PAE
    # ------------------------------------------------

    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4


    # ------------------------------------------------
    # Enable Long Mode through EFER MSR
    # ------------------------------------------------

    mov $0xC0000080, %ecx
    rdmsr

    or $0x100, %eax

    wrmsr


    # ------------------------------------------------
    # Enable Paging
    # ------------------------------------------------

    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0


    # ------------------------------------------------
    # Far jump into 64-bit code segment
    # ------------------------------------------------

    ljmp $0x08, $long_mode_start


# ====================================================
# 64-bit mode
# ====================================================

.code64

long_mode_start:

    # Load data segment
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

    # Clear registers
    xor %eax, %eax
    xor %ecx, %ecx
    xor %edx, %edx

    # Stack
    mov $stack_top, %rsp

    # Call C kernel
    call kernel_main


.hang:
    hlt
    jmp .hang


# ====================================================
# GDT
# ====================================================

.align 8

gdt64:

    # Null descriptor
    .quad 0x0000000000000000

    # 64-bit code segment
    .quad 0x00AF9A000000FFFF

    # Data segment
    .quad 0x00AF92000000FFFF

gdt64_end:

gdt64_descriptor:
    .word gdt64_end - gdt64 - 1
    .long gdt64


# ====================================================
# Multiboot information pointer
# ====================================================

.align 8

multiboot_info:
    .long 0


# ====================================================
# Page tables
# ====================================================

.align 4096

page_table_l4:
    .space 4096

.align 4096

page_table_l3:
    .space 4096

.align 4096

page_table_l2:
    .space 4096


# ====================================================
# Stack
# ====================================================

.align 16

stack_bottom:
    .space 16384

stack_top: