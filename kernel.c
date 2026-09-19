#include <stdint.h>
#include "graphics.h"

static void debug_char(char c)
{
    __asm__ volatile(
        "outb %0, $0xE9"
        :
        : "a"(c));
}

static void debug_string(const char *s)
{
    while (*s)
    {
        debug_char(*s++);
    }
}

void kernel_main(uint32_t mbi_addr)
{
    debug_string("A: kernel_main\n");

    int result = graphics_init(mbi_addr);

    debug_string("B: graphics_init returned\n");

    if (result != 0)
    {
        debug_string("C: graphics_init FAILED\n");

        while (1)
            __asm__ volatile("hlt");
    }

    debug_string("D: graphics_init OK\n");

    graphics_clear(COLOR_BLACK);

    int y_dim = graphics_get_height();
    int x_dim = graphics_get_width();

    int b_x = x_dim / 2;
    int b_y = y_dim / 2;

    graphics_draw_rect(x_dim / 2 - (b_x / 2), y_dim / 2 - (b_y / 2), b_x, b_y, COLOR_RED);
    graphics_fill_rect(x_dim / 2 - (b_x / 2) + 1, y_dim / 2 - (b_y / 2) + 1, b_x - 2, b_y - 2, COLOR_BLUE);

    graphics_draw_string(10,10,"Hello World!",COLOR_GREEN);
    graphics_draw_string(10,23,"Project-MihateOS",COLOR_GREEN);

    debug_string("F: put_pixel OK\n");

    while (1)
        __asm__ volatile("hlt");
}