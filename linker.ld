ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot ALIGN(8) :
    {
        KEEP(*(.multiboot))
    }

    .text ALIGN(16) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(16) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(16) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4096) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}