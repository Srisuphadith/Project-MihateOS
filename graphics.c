#include "graphics.h"
#include "font8x8.h"
#include <stdint.h>


/*
 * ---------------------------------------------------------
 * Multiboot2 definitions
 * ---------------------------------------------------------
 */

#define MULTIBOOT_TAG_TYPE_END         0
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2


/*
 * ---------------------------------------------------------
 * Multiboot2 framebuffer structures
 * ---------------------------------------------------------
 *
 * The framebuffer tag starts with this common structure.
 *
 * Multiboot2:
 *
 * offset  0 : type
 * offset  4 : size
 * offset  8 : framebuffer address
 * offset 16 : pitch
 * offset 20 : width
 * offset 24 : height
 * offset 28 : bpp
 * offset 29 : framebuffer type
 * offset 30 : reserved
 *
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
 * RGB framebuffer tag.
 *
 * The RGB information starts at offset 32.
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
 * ---------------------------------------------------------
 * Graphics state
 * ---------------------------------------------------------
 */

static volatile uint8_t *framebuffer = 0;

static uint32_t framebuffer_width  = 0;
static uint32_t framebuffer_height = 0;
static uint32_t framebuffer_pitch  = 0;

static uint8_t framebuffer_bpp = 0;
static uint8_t framebuffer_bytes_per_pixel = 0;


/*
 * RGB channel layout
 */

static uint8_t red_position   = 0;
static uint8_t red_mask_size  = 0;

static uint8_t green_position = 0;
static uint8_t green_mask_size = 0;

static uint8_t blue_position  = 0;
static uint8_t blue_mask_size = 0;


/*
 * ---------------------------------------------------------
 * Helper: create channel mask
 * ---------------------------------------------------------
 */

static uint32_t channel_mask(uint8_t bits)
{
    if (bits == 0)
    {
        return 0;
    }

    if (bits >= 32)
    {
        return 0xFFFFFFFFu;
    }

    return (1u << bits) - 1u;
}


/*
 * ---------------------------------------------------------
 * Helper: convert 8-bit color to framebuffer channel size
 * ---------------------------------------------------------
 *
 * Example:
 *
 * 8-bit color -> 8-bit channel
 *
 * 255 -> 255
 *
 *
 * 8-bit color -> 5-bit channel
 *
 * 255 -> 31
 *
 */

static uint32_t scale_color(
    uint8_t color,
    uint8_t mask_size
)
{
    if (mask_size == 0)
    {
        return 0;
    }

    if (mask_size >= 8)
    {
        return color;
    }

    uint32_t max_value =
        channel_mask(mask_size);

    return
        ((uint32_t)color * max_value + 127) / 255;
}



static void debug_char(char c)
{
    __asm__ volatile (
        "outb %0, $0xE9"
        :
        : "a"((uint8_t)c)
    );
}


static void debug_string(const char *s)
{
    while (*s)
    {
        debug_char(*s++);
    }
}


static void debug_hex(uint32_t value)
{
    const char hex[] = "0123456789ABCDEF";

    debug_string("0x");

    for (int i = 7; i >= 0; i--)
    {
        debug_char(
            hex[(value >> (i * 4)) & 0xF]
        );
    }

    debug_char('\n');
}
/*
 * ---------------------------------------------------------
 * graphics_init()
 * ---------------------------------------------------------
 */

// int graphics_init(uint32_t mbi_addr)
// {
//     /*
//      * Reset state
//      */

//     framebuffer = 0;

//     framebuffer_width = 0;
//     framebuffer_height = 0;
//     framebuffer_pitch = 0;

//     framebuffer_bpp = 0;
//     framebuffer_bytes_per_pixel = 0;

//     red_position = 0;
//     red_mask_size = 0;

//     green_position = 0;
//     green_mask_size = 0;

//     blue_position = 0;
//     blue_mask_size = 0;


//     /*
//      * Multiboot2 information:
//      *
//      * +0 : total size
//      * +4 : reserved
//      * +8 : first tag
//      */

//     uint8_t *mbi =
//         (uint8_t *)(uintptr_t)mbi_addr;

//     uint8_t *tag_addr =
//         mbi + 8;


//     while (1)
//     {
//         uint32_t type =
//             *(uint32_t *)(tag_addr + 0);

//         uint32_t size =
//             *(uint32_t *)(tag_addr + 4);


//         /*
//          * End tag
//          */

//         if (type == MULTIBOOT_TAG_TYPE_END)
//         {
//             break;
//         }


//         /*
//          * Framebuffer tag
//          */

//         if (type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
//         {
//             framebuffer_rgb_t *fb =
//                 (framebuffer_rgb_t *)tag_addr;


//             /*
//              * We currently support
//              * RGB framebuffer only.
//              */

//             if (fb->common.framebuffer_type !=
//                 MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
//             {
//                 return -2;
//             }


//             /*
//              * Validate bits per pixel.
//              *
//              * We need at least one byte per pixel.
//              */

//             if (fb->common.framebuffer_bpp == 0)
//             {
//                 return -3;
//             }


//             /*
//              * Calculate bytes per pixel.
//              *
//              * Examples:
//              *
//              * 32 bpp -> 4 bytes
//              * 24 bpp -> 3 bytes
//              * 16 bpp -> 2 bytes
//              *  8 bpp -> 1 byte
//              */

//             framebuffer_bytes_per_pixel =
//                 (fb->common.framebuffer_bpp + 7) / 8;


//             /*
//              * Save framebuffer information.
//              */

//             framebuffer =
//                 (volatile uint8_t *)
//                 (uintptr_t)
//                 fb->common.framebuffer_addr;

//             framebuffer_width =
//                 fb->common.framebuffer_width;

//             framebuffer_height =
//                 fb->common.framebuffer_height;

//             framebuffer_pitch =
//                 fb->common.framebuffer_pitch;

//             framebuffer_bpp =
//                 fb->common.framebuffer_bpp;


//             /*
//              * Save RGB layout.
//              */

//             red_position =
//                 fb->red_position;

//             red_mask_size =
//                 fb->red_mask_size;

//             green_position =
//                 fb->green_position;

//             green_mask_size =
//                 fb->green_mask_size;

//             blue_position =
//                 fb->blue_position;

//             blue_mask_size =
//                 fb->blue_mask_size;


//             /*
//              * Basic validation.
//              */

//             if (framebuffer == 0)
//             {
//                 return -4;
//             }

//             if (framebuffer_width == 0 ||
//                 framebuffer_height == 0)
//             {
//                 return -5;
//             }

//             if (framebuffer_pitch == 0)
//             {
//                 return -6;
//             }

//             if (framebuffer_bytes_per_pixel == 0)
//             {
//                 return -7;
//             }


//             /*
//              * Make sure the pitch can contain
//              * at least one complete row.
//              */

//             uint64_t minimum_pitch =
//                 (uint64_t)framebuffer_width *
//                 framebuffer_bytes_per_pixel;

//             if ((uint64_t)framebuffer_pitch <
//                 minimum_pitch)
//             {
//                 return -8;
//             }


//             return 0;
//         }


//         /*
//          * Multiboot2 tags are aligned to 8 bytes.
//          */

//         if (size < 8)
//         {
//             return -9;
//         }

//         size =
//             (size + 7) & ~7u;

//         tag_addr += size;
//     }


//     /*
//      * No framebuffer tag found.
//      */

//     return -1;
// }
int graphics_init(uint32_t mbi_addr)
{
    /*
     * Reset state
     */

    framebuffer = 0;

    framebuffer_width = 0;
    framebuffer_height = 0;
    framebuffer_pitch = 0;

    framebuffer_bpp = 0;
    framebuffer_bytes_per_pixel = 0;

    red_position = 0;
    red_mask_size = 0;

    green_position = 0;
    green_mask_size = 0;

    blue_position = 0;
    blue_mask_size = 0;


    /*
     * Multiboot2 information:
     *
     * +0 : total size
     * +4 : reserved
     * +8 : first tag
     */

    uint8_t *mbi =
        (uint8_t *)(uintptr_t)mbi_addr;

    uint8_t *tag_addr =
        mbi + 8;


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
             * We currently support
             * RGB framebuffer only.
             */

            if (fb->common.framebuffer_type !=
                MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
            {
                return -2;
            }


            /*
             * Validate bits per pixel.
             *
             * We need at least one byte per pixel.
             */

            if (fb->common.framebuffer_bpp == 0)
            {
                return -3;
            }


            /*
             * Calculate bytes per pixel.
             *
             * Examples:
             *
             * 32 bpp -> 4 bytes
             * 24 bpp -> 3 bytes
             * 16 bpp -> 2 bytes
             *  8 bpp -> 1 byte
             */

            framebuffer_bytes_per_pixel =
                (fb->common.framebuffer_bpp + 7) / 8;


            /*
             * Save framebuffer information.
             */

            framebuffer =
                (volatile uint8_t *)
                (uintptr_t)
                fb->common.framebuffer_addr;

            framebuffer_width =
                fb->common.framebuffer_width;

            framebuffer_height =
                fb->common.framebuffer_height;

            framebuffer_pitch =
                fb->common.framebuffer_pitch;

            framebuffer_bpp =
                fb->common.framebuffer_bpp;


            /*
             * Save RGB layout.
             */

            red_position =
                fb->red_position;

            red_mask_size =
                fb->red_mask_size;

            green_position =
                fb->green_position;

            green_mask_size =
                fb->green_mask_size;

            blue_position =
                fb->blue_position;

            blue_mask_size =
                fb->blue_mask_size;


            /*
             * DEBUG framebuffer information
             */

            debug_string("FB addr: ");
            debug_hex(
                (uint32_t)fb->common.framebuffer_addr
            );

            debug_string("FB width: ");
            debug_hex(framebuffer_width);

            debug_string("FB height: ");
            debug_hex(framebuffer_height);

            debug_string("FB pitch: ");
            debug_hex(framebuffer_pitch);

            debug_string("FB bpp: ");
            debug_hex(framebuffer_bpp);

            debug_string("FB bytes/pixel: ");
            debug_hex(framebuffer_bytes_per_pixel);

            debug_string("FB type: ");
            debug_hex(fb->common.framebuffer_type);

            debug_string("R pos: ");
            debug_hex(red_position);

            debug_string("R mask: ");
            debug_hex(red_mask_size);

            debug_string("G pos: ");
            debug_hex(green_position);

            debug_string("G mask: ");
            debug_hex(green_mask_size);

            debug_string("B pos: ");
            debug_hex(blue_position);

            debug_string("B mask: ");
            debug_hex(blue_mask_size);


            /*
             * Basic validation.
             */

            if (framebuffer == 0)
            {
                return -4;
            }

            if (framebuffer_width == 0 ||
                framebuffer_height == 0)
            {
                return -5;
            }

            if (framebuffer_pitch == 0)
            {
                return -6;
            }

            if (framebuffer_bytes_per_pixel == 0)
            {
                return -7;
            }


            /*
             * Make sure the pitch can contain
             * at least one complete row.
             */

            uint64_t minimum_pitch =
                (uint64_t)framebuffer_width *
                framebuffer_bytes_per_pixel;

            if ((uint64_t)framebuffer_pitch <
                minimum_pitch)
            {
                return -8;
            }


            /*
             * Framebuffer successfully initialized.
             */

            return 0;
        }


        /*
         * Multiboot2 tags are aligned to 8 bytes.
         */

        if (size < 8)
        {
            return -9;
        }

        size =
            (size + 7) & ~7u;

        tag_addr += size;
    }


    /*
     * No framebuffer tag found.
     */

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
 * Convert Color -> framebuffer pixel
 * ---------------------------------------------------------
 */

static uint32_t make_pixel(Color color)
{
    uint32_t r =
        scale_color(
            color.r,
            red_mask_size
        );

    uint32_t g =
        scale_color(
            color.g,
            green_mask_size
        );

    uint32_t b =
        scale_color(
            color.b,
            blue_mask_size
        );


    uint32_t pixel = 0;


    /*
     * Apply channel masks before shifting.
     */

    r &= channel_mask(red_mask_size);
    g &= channel_mask(green_mask_size);
    b &= channel_mask(blue_mask_size);


    pixel |=
        r << red_position;

    pixel |=
        g << green_position;

    pixel |=
        b << blue_position;


    return pixel;
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
     * Check framebuffer.
     */

    if (framebuffer == 0)
    {
        return;
    }


    /*
     * Bounds checking.
     */

    if (x >= framebuffer_width ||
        y >= framebuffer_height)
    {
        return;
    }


    /*
     * Construct pixel.
     */

    uint32_t pixel =
        make_pixel(color);


    /*
     * Calculate address.
     *
     * IMPORTANT:
     *
     * Do NOT assume 4 bytes/pixel.
     */

    volatile uint8_t *pixel_addr =
        framebuffer +
        ((uint32_t)y * framebuffer_pitch) +
        ((uint32_t)x * framebuffer_bytes_per_pixel);


    /*
     * Write according to pixel size.
     */

    switch (framebuffer_bytes_per_pixel)
    {
        case 1:
            *(volatile uint8_t *)pixel_addr =
                (uint8_t)pixel;
            break;


        case 2:
            *(volatile uint16_t *)pixel_addr =
                (uint16_t)pixel;
            break;


        case 3:
        {
            /*
             * 24-bit framebuffer.
             *
             * Write three bytes explicitly.
             */

            pixel_addr[0] =
                (uint8_t)(pixel & 0xFF);

            pixel_addr[1] =
                (uint8_t)((pixel >> 8) & 0xFF);

            pixel_addr[2] =
                (uint8_t)((pixel >> 16) & 0xFF);

            break;
        }


        case 4:
            *(volatile uint32_t *)pixel_addr =
                pixel;
            break;


        default:
            /*
             * Unsupported framebuffer size.
             */

            break;
    }
}
// void graphics_put_pixel(
//     uint32_t x,
//     uint32_t y,
//     Color color
// )
// {
//     if (x >= framebuffer_width ||
//         y >= framebuffer_height)
//     {
//         return;
//     }

//     uint32_t offset =
//         y * framebuffer_pitch +
//         x * 3;

//     volatile uint8_t *pixel =
//         framebuffer + offset;

//     pixel[0] = color.b;
//     pixel[1] = color.g;
//     pixel[2] = color.r;
// }

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
     * Empty rectangle.
     */

    if (width == 0 ||
        height == 0)
    {
        return;
    }


    /*
     * Completely outside framebuffer.
     */

    if (x >= framebuffer_width ||
        y >= framebuffer_height)
    {
        return;
    }


    /*
     * Clip width.
     *
     * Avoid:
     *
     * x + width
     *
     * overflowing uint32_t.
     */

    uint32_t max_width =
        framebuffer_width - x;

    if (width > max_width)
    {
        width = max_width;
    }


    /*
     * Clip height.
     */

    uint32_t max_height =
        framebuffer_height - y;

    if (height > max_height)
    {
        height = max_height;
    }


    /*
     * Draw.
     */

    for (uint32_t yy = 0;
         yy < height;
         yy++)
    {
        for (uint32_t xx = 0;
             xx < width;
             xx++)
        {
            graphics_put_pixel(
                x + xx,
                y + yy,
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
    if (width == 0 ||
        height == 0)
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

    if (height > 1)
    {
        graphics_fill_rect(
            x,
            y + height - 1,
            width,
            1,
            color
        );
    }


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

    if (width > 1)
    {
        graphics_fill_rect(
            x + width - 1,
            y,
            1,
            height,
            color
        );
    }
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
        x1 > x0
            ? x1 - x0
            : x0 - x1;

    int sx =
        x0 < x1
            ? 1
            : -1;

    int dy =
        y1 > y0
            ? y0 - y1
            : y1 - y0;

    int sy =
        y0 < y1
            ? 1
            : -1;

    int error =
        dx + dy;


    while (1)
    {
        /*
         * graphics_put_pixel()
         * already performs bounds checking.
         */

        if (x0 >= 0 &&
            y0 >= 0)
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


void graphics_draw_char(
    uint32_t x,
    uint32_t y,
    char c,
    Color color
)
{
    const uint8_t *glyph = font8x8[(uint8_t)c];

    for (uint32_t row = 0; row < 8; row++)
    {
        for (uint32_t col = 0; col < 8; col++)
        {
            if (glyph[row] & (1 << (7 - col)))
            {
                graphics_put_pixel(
                    x + col,
                    y + row,
                    color
                );
            }
        }
    }
}

void graphics_draw_string(
    uint32_t x,
    uint32_t y,
    char *c,
    Color color
){
    int cnt = 0;
    int offset = x;
    while (c[cnt] != '\0')
    {
        graphics_draw_char(offset,y,c[cnt],color);
        cnt++;
        offset += 8;
    }
}