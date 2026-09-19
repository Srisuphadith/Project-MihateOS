#include <stdint.h>
#include "idt.h"
extern void irq1_stub(void);
struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtr;

static void idt_set_gate(
    uint8_t vector,
    uint32_t handler
)
{
    idt[vector].offset_low =
        handler & 0xFFFF;

    idt[vector].selector =
        0x08;

    idt[vector].zero =
        0;

    idt[vector].type_attr =
        0x8E;

    idt[vector].offset_high =
        (handler >> 16) & 0xFFFF;
}

void idt_init(void)
{
    for (int i = 0; i < 256; i++)
    {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    idt_set_gate(
        0x21,
        (uint32_t)irq1_stub
    );

    idtr.limit =
        sizeof(idt) - 1;

    idtr.base =
        (uint32_t)&idt[0];

    asm volatile (
        "lidt %0"
        :
        : "m"(idtr)
    );
}