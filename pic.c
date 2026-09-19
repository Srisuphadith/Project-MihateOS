#include <stdint.h>

#include "io.h"
#include "pic.h"

#define PIC1_COMMAND  0x20
#define PIC1_DATA     0x21

#define PIC2_COMMAND  0xA0
#define PIC2_DATA     0xA1

void pic_init(void)
{
    /*
     * ICW1
     */
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    /*
     * ICW2
     */
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    /*
     * ICW3
     */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    /*
     * ICW4
     */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /*
     * Mask everything
     */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}