#include "graphics.h"


/*
 * Multiboot2 tag types
 */
#define MULTIBOOT_TAG_TYPE_END         0
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1


/*
 * Multiboot2 framebuffer tag
 *
 * This is the common part of the framebuffer tag.
 */
typedef struct
{
    uint32_t type;
    uint32_t size;

    uint64_t framebuffer_addr;

    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;

    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;

    uint16_t reserved;

} framebuffer_common_t;


/*
 * RGB framebuffer information
 */
typedef struct
{
    framebuffer_common_t common;

    uint8_t red_position;
    uint8_t red_mask_size;

    uint8_t green_position;
    uint8_t green_mask_size;

    uint8_t blue_position;
    uint8_t blue_mask_size;

} framebuffer_rgb_t;


/*
 * Graphics state
 */
static volatile uint8_t *framebuffer = 0;

static uint32_t framebuffer_width  = 0;
static uint32_t framebuffer_height = 0;
static uint32_t framebuffer_pitch  = 0;

static uint8_t framebuffer_bpp = 0;

static uint8_t red_position   = 0;
static uint8_t green_position = 0;
static uint8_t blue_position  = 0;


/*
 * ---------------------------------------------------------
 * graphics_init()
 * ---------------------------------------------------------
 */

int graphics_init(uint32_t mbi_addr)
{
    uint8_t *mbi =
        (uint8_t *)(uintptr_t)mbi_addr;


    /*
     * Multiboot2 information structure:
     *
     * offset 0 = total size
     * offset 4 = reserved
     * offset 8 = first tag
     */

    uint8_t *tag_addr = mbi + 8;


    while (1)
    {
        uint32_t type =
            *(uint32_t *)(tag_addr + 0);

        uint32_t size =
            *(uint32_t *)(tag_addr + 4);


        /*
         * End tag
         */
        if (type == MULTIBOOT_TAG_TYPE_END)
        {
            break;
        }


        /*
         * Framebuffer tag
         */
        if (type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
        {
            framebuffer_rgb_t *fb =
                (framebuffer_rgb_t *)tag_addr;


            /*
             * We only support RGB framebuffer
             */
            if (fb->common.framebuffer_type !=
                MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
            {
                return -2;
            }


            /*
             * Save framebuffer information
             */
            framebuffer =
                (volatile uint8_t *)
                (uintptr_t)fb->common.framebuffer_addr;

            framebuffer_width =
                fb->common.framebuffer_width;

            framebuffer_height =
                fb->common.framebuffer_height;

            framebuffer_pitch =
                fb->common.framebuffer_pitch;

            framebuffer_bpp =
                fb->common.framebuffer_bpp;


            red_position =
                fb->red_position;

            green_position =
                fb->green_position;

            blue_position =
                fb->blue_position;


            /*
             * Currently our renderer assumes
             * 32-bit framebuffer.
             */
            if (framebuffer_bpp != 32)
            {
                return -2;
            }


            return 0;
        }


        /*
         * Multiboot2 tags are 8-byte aligned.
         */
        size = (size + 7) & ~7;

        tag_addr += size;
    }


    return -1;
}


/*
 * ---------------------------------------------------------
 * Getters
 * ---------------------------------------------------------
 */

uint32_t graphics_get_width(void)
{
    return framebuffer_width;
}


uint32_t graphics_get_height(void)
{
    return framebuffer_height;
}


uint32_t graphics_get_pitch(void)
{
    return framebuffer_pitch;
}


uint8_t graphics_get_bpp(void)
{
    return framebuffer_bpp;
}


/*
 * ---------------------------------------------------------
 * graphics_put_pixel()
 * ---------------------------------------------------------
 */

void graphics_put_pixel(
    uint32_t x,
    uint32_t y,
    Color color
)
{
    /*
     * Bounds checking
     */
    if (x >= framebuffer_width ||
        y >= framebuffer_height)
    {
        return;
    }


    /*
     * Construct pixel value according
     * to framebuffer RGB layout.
     */
    uint32_t pixel =
        ((uint32_t)color.r << red_position) |
        ((uint32_t)color.g << green_position) |
        ((uint32_t)color.b << blue_position);


    /*
     * Framebuffer address:
     *
     * framebuffer
     *     +
     *     y * pitch
     *     +
     *     x * bytes_per_pixel
     *
     * 32-bit = 4 bytes/pixel
     */

    volatile uint32_t *pixel_addr =
        (volatile uint32_t *)
        (
            framebuffer +
            y * framebuffer_pitch +
            x * 4
        );


    *pixel_addr = pixel;
}


/*
 * ---------------------------------------------------------
 * graphics_clear()
 * ---------------------------------------------------------
 */

void graphics_clear(Color color)
{
    for (uint32_t y = 0;
         y < framebuffer_height;
         y++)
    {
        for (uint32_t x = 0;
             x < framebuffer_width;
             x++)
        {
            graphics_put_pixel(
                x,
                y,
                color
            );
        }
    }
}


/*
 * ---------------------------------------------------------
 * graphics_fill_rect()
 * ---------------------------------------------------------
 */

void graphics_fill_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    Color color
)
{
    /*
     * Clip rectangle to screen.
     */

    if (x >= framebuffer_width ||
        y >= framebuffer_height)
    {
        return;
    }


    if (x + width > framebuffer_width)
    {
        width =
            framebuffer_width - x;
    }


    if (y + height > framebuffer_height)
    {
        height =
            framebuffer_height - y;
    }


    for (uint32_t yy = y;
         yy < y + height;
         yy++)
    {
        for (uint32_t xx = x;
             xx < x + width;
             xx++)
        {
            graphics_put_pixel(
                xx,
                yy,
                color
            );
        }
    }
}


/*
 * ---------------------------------------------------------
 * graphics_draw_rect()
 * ---------------------------------------------------------
 */

void graphics_draw_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    Color color
)
{
    if (width == 0 || height == 0)
    {
        return;
    }


    /*
     * Top
     */
    graphics_fill_rect(
        x,
        y,
        width,
        1,
        color
    );


    /*
     * Bottom
     */
    graphics_fill_rect(
        x,
        y + height - 1,
        width,
        1,
        color
    );


    /*
     * Left
     */
    graphics_fill_rect(
        x,
        y,
        1,
        height,
        color
    );


    /*
     * Right
     */
    graphics_fill_rect(
        x + width - 1,
        y,
        1,
        height,
        color
    );
}


/*
 * ---------------------------------------------------------
 * graphics_draw_line()
 * ---------------------------------------------------------
 *
 * Bresenham line algorithm
 *
 */

void graphics_draw_line(
    int x0,
    int y0,
    int x1,
    int y1,
    Color color
)
{
    int dx =
        x1 > x0 ? x1 - x0 : x0 - x1;

    int sx =
        x0 < x1 ? 1 : -1;

    int dy =
        y1 > y0 ? y0 - y1 : y1 - y0;

    int sy =
        y0 < y1 ? 1 : -1;

    int error =
        dx + dy;


    while (1)
    {
        /*
         * Only draw if coordinate is positive.
         */
        if (x0 >= 0 && y0 >= 0)
        {
            graphics_put_pixel(
                (uint32_t)x0,
                (uint32_t)y0,
                color
            );
        }


        if (x0 == x1 &&
            y0 == y1)
        {
            break;
        }


        int e2 =
            2 * error;


        if (e2 >= dy)
        {
            error += dy;
            x0 += sx;
        }


        if (e2 <= dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}