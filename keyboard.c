#include <stdint.h>

#include "io.h"
#include "keyboard.h"
#include "graphics.h"

static const char scancode_table[128] =
{
    0,

    27,

    '1','2','3','4','5','6','7','8','9','0',
    '-','=',
    '\b',
    '\t',

    'q','w','e','r','t','y','u','i','o','p',
    '[',']',
    '\n',

    0,

    'a','s','d','f','g','h','j','k','l',
    ';','\'','`',

    0,
    '\\',

    'z','x','c','v','b','n','m',
    ',','.','/',

    0,
    '*',
    0,
    ' '
};
int baseX = 10;
void keyboard_handler(void)
{
    uint8_t scancode =
        inb(0x60);

    /*
     * High bit = key release
     */
    if (!(scancode & 0x80))
    {
        if (scancode < 128)
        {
            char c =
                scancode_table[scancode];

            if (c)
            {
                outb(0xE9, c);
                /*
                 * ตอนแรกยังไม่ต้อง console
                 * เอาไว้ debug ก่อน
                 */
                graphics_draw_char(baseX,50,c,COLOR_WHITE);
                baseX += 8;
            }
        }
    }

    /*
     * End Of Interrupt
     */
    outb(0x20, 0x20);
}