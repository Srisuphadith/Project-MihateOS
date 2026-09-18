#include <stdint.h>

void kernel_main(void)
{
    volatile uint16_t *vga =
        (volatile uint16_t *)0xB8000;

    const char *message = "Hello, MyOS!";

    for (int i = 0; message[i] != '\0'; i++) {
        vga[i] =
            (uint16_t)message[i]
            | ((uint16_t)0x07 << 8);
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}