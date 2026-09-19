#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>


/*
 * RGB color
 */
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

} Color;


/*
 * Common colors
 */
#define COLOR_BLACK   ((Color){ 0,   0,   0   })
#define COLOR_WHITE   ((Color){ 255, 255, 255 })
#define COLOR_RED     ((Color){ 255, 0,   0   })
#define COLOR_GREEN   ((Color){ 0,   255, 0   })
#define COLOR_BLUE    ((Color){ 0,   0,   255 })
#define COLOR_YELLOW  ((Color){ 255, 255, 0   })


/*
 * Initialize graphics subsystem
 *
 * mbi_addr:
 *     Multiboot2 information structure address
 *
 * return:
 *     0  = success
 *    -1  = framebuffer not found
 *    -2  = unsupported framebuffer
 */
int graphics_init(uint32_t mbi_addr);


/*
 * Screen information
 */
uint32_t graphics_get_width(void);
uint32_t graphics_get_height(void);
uint32_t graphics_get_pitch(void);
uint8_t  graphics_get_bpp(void);


/*
 * Pixel
 */
void graphics_put_pixel(
    uint32_t x,
    uint32_t y,
    Color color
);


/*
 * Clear entire screen
 */
void graphics_clear(Color color);


/*
 * Filled rectangle
 */
void graphics_fill_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    Color color
);


/*
 * Rectangle outline
 */
void graphics_draw_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    Color color
);


/*
 * Line
 */
void graphics_draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    Color color
);

void graphics_draw_char(
    uint32_t x,
    uint32_t y,
    char c,
    Color color
);
void graphics_draw_string(
    uint32_t x,
    uint32_t y,
    char *c,
    Color color
);

#endif