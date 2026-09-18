#include <stdint.h>
#include "graphics.h"


void kernel_main(uint32_t mbi_addr)
{
    /*
     * Initialize graphics
     */
    if (graphics_init(mbi_addr) != 0)
    {
        /*
         * Graphics initialization failed.
         */
        while (1)
        {
            __asm__ volatile ("hlt");
        }
    }


    /*
     * Clear screen
     */
    graphics_clear(
        COLOR_BLACK
    );


    /*
     * Get screen size
     */
    uint32_t width =
        graphics_get_width();

    uint32_t height =
        graphics_get_height();


    /*
     * Large rectangle in center
     */
    uint32_t rect_width = 800;
    uint32_t rect_height = 400;

    uint32_t rect_x =
        (width - rect_width) / 2;

    uint32_t rect_y =
        (height - rect_height) / 2;


    graphics_fill_rect(
        rect_x,
        rect_y,
        rect_width,
        rect_height,
        COLOR_BLUE
    );


    /*
     * Rectangle outline
     */
    graphics_draw_rect(
        rect_x - 10,
        rect_y - 10,
        rect_width + 20,
        rect_height + 20,
        COLOR_WHITE
    );


    /*
     * Diagonal lines
     */
    graphics_draw_line(
        0,
        0,
        width - 1,
        height - 1,
        COLOR_RED
    );


    graphics_draw_line(
        width - 1,
        0,
        0,
        height - 1,
        COLOR_GREEN
    );


    /*
     * Kernel idle
     */
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}