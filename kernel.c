#include <stdint.h>

#define MULTIBOOT_TAG_TYPE_END         0
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

#define FRAMEBUFFER_TYPE_RGB 1


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

} multiboot_tag_framebuffer_common_t;


typedef struct
{
    multiboot_tag_framebuffer_common_t common;

    uint8_t red_position;
    uint8_t red_mask_size;

    uint8_t green_position;
    uint8_t green_mask_size;

    uint8_t blue_position;
    uint8_t blue_mask_size;

} framebuffer_info_t;


/*
 * Global framebuffer information
 */

static volatile uint8_t *framebuffer;

static uint32_t framebuffer_pitch;
static uint32_t framebuffer_width;
static uint32_t framebuffer_height;
static uint8_t framebuffer_bpp;

static uint8_t red_position;
static uint8_t green_position;
static uint8_t blue_position;


/*
 * Find framebuffer supplied by GRUB
 */

static int framebuffer_init(uint32_t mbi_addr)
{
    uint8_t *addr = (uint8_t *)(uintptr_t)mbi_addr;

    uint8_t *tag_addr = addr + 8;

    while (1)
    {
        uint32_t type =
            *(uint32_t *)(tag_addr + 0);

        uint32_t size =
            *(uint32_t *)(tag_addr + 4);

        if (type == MULTIBOOT_TAG_TYPE_END)
        {
            break;
        }

        if (type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
        {
            framebuffer_info_t *fb =
                (framebuffer_info_t *)tag_addr;

            if (fb->common.framebuffer_type !=
                FRAMEBUFFER_TYPE_RGB)
            {
                return -1;
            }

            framebuffer =
                (volatile uint8_t *)
                (uintptr_t)fb->common.framebuffer_addr;

            framebuffer_pitch =
                fb->common.framebuffer_pitch;

            framebuffer_width =
                fb->common.framebuffer_width;

            framebuffer_height =
                fb->common.framebuffer_height;

            framebuffer_bpp =
                fb->common.framebuffer_bpp;

            red_position =
                fb->red_position;

            green_position =
                fb->green_position;

            blue_position =
                fb->blue_position;

            return 0;
        }

        size = (size + 7) & ~7;

        tag_addr += size;
    }

    return -1;
}


/*
 * Put one pixel
 */

static void put_pixel(
    uint32_t x,
    uint32_t y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (x >= framebuffer_width ||
        y >= framebuffer_height)
    {
        return;
    }

    uint32_t pixel =
        ((uint32_t)r << red_position) |
        ((uint32_t)g << green_position) |
        ((uint32_t)b << blue_position);

    volatile uint32_t *pixel_addr =
        (volatile uint32_t *)
        (framebuffer +
         y * framebuffer_pitch +
         x * 4);

    *pixel_addr = pixel;
}


/*
 * Draw rectangle
 */

static void draw_rect(
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    for (uint32_t yy = y;
         yy < y + height;
         yy++)
    {
        for (uint32_t xx = x;
             xx < x + width;
             xx++)
        {
            put_pixel(
                xx,
                yy,
                r,
                g,
                b
            );
        }
    }
}


/*
 * Kernel
 */

void kernel_main(uint32_t mbi_addr)
{
    if (framebuffer_init(mbi_addr) != 0)
    {
        /*
         * No framebuffer
         */
        while (1)
        {
            __asm__ volatile ("hlt");
        }
    }


    /*
     * Background
     */
    for (uint32_t y = 0;
         y < framebuffer_height;
         y++)
    {
        for (uint32_t x = 0;
             x < framebuffer_width;
             x++)
        {
            put_pixel(
                x,
                y,
                10,
                10,
                18
            );
        }
    }


    /*
     * Center rectangle
     */

    uint32_t width = 800;
    uint32_t height = 400;

    uint32_t x =
        (framebuffer_width - width) / 2;

    uint32_t y =
        (framebuffer_height - height) / 2;

    draw_rect(
        x,
        y,
        width,
        height,
        30,
        120,
        220
    );


    while (1)
    {
        __asm__ volatile ("hlt");
    }
}