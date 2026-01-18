#include "drivers/display_old.h"
#include "assets/font.h"

// Display driver initialization status
bool display_Initialize = false;

// CLI video memory base address
uint16_t* display_CLIVideoMemory = (uint16_t*)DISPLAY_CLIVIDEOMEMORY;
// GUI video memory base address
uint8_t* display_GUIVideoMemory = (uint8_t*)DISPLAY_GUIVIDEOMEMORY;

// Display text memory
char display_TextMemory[DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT];

// Display video memory
uint8_t display_VideoMemory[DISPLAY_GUIWIDTH][DISPLAY_GUIHEIGHT];

// CLI display background color
color_t display_CLIBackColor = DISPLAY_CLIDEFAULT_BACKCOLOR;
// CLI display text color
color_t display_CLITextColor = DISPLAY_CLIDEFAULT_TEXTCOLOR;

/**
 * @brief Function for clear entire screen (Available in both modes)
 */
void display_clear() {
    if (!display_Initialize) { return; }
    if (!KERNEL_GRAPHICMODE) {
        for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) {
            display_CLIVideoMemory[i] =
                (uint16_t)(display_CLIBackColor << 4 | display_CLITextColor << 8) | (uint16_t)0x0000;
        }
    } else {
        fill(&display_TextMemory, 0, DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT);
        fill(&display_VideoMemory, 0, DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT); return;
        for (int i = 0; i < DISPLAY_GUIHEIGHT; ++i) {
            for (int j = 0; j < DISPLAY_GUIWIDTH; ++j) {
                // uint8_t* pixel = display_GUIVideoMemory + DISPLAY_GUIWIDTH * i + j; *pixel = 0x00;
                display_VideoMemory[j][i] = 0x00;
            }
        }
    }
}

/**
 * @brief Function for print a single character at a specific position on the screen (Available in CLI mode)
 * 
 * @param chr Character to be print
 * @param ptr Specific screen position
 */
void display_putchar(char chr, int ptr) {
    if (!display_Initialize) { return; }
    if (!KERNEL_GRAPHICMODE) {
        if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return; }
        display_CLIVideoMemory[ptr] =
            (uint16_t)(display_CLIBackColor << 4 | display_CLITextColor << 8) | (uint16_t)chr;
    } else { // ! NOTE: This code in development
        display_TextMemory[ptr] = chr; return;
        int x = (ptr % DISPLAY_CLIWIDTH) * 8;
        int y = (ptr / DISPLAY_CLIWIDTH) * 10;
        if (chr < 0 || chr > 127) return;
        for (int row = 0; row < DISPLAY_FONTHEIGHT; row++) {
            char row_bits = font8x8_basic[(int)chr][row];
            for (int col = 0; col < DISPLAY_FONTWIDTH; col++) {
                int bit = (row_bits >> col) & 1;
                uint32_t color = bit ? 0x07 : 0x00;
                display_putpixel(x + col, y + row, color);
            }
        }
        // int idx = chr * 10;
        // for (int row = 0; row < 10; row++) {
        //     unsigned char bits = font_8x10[idx + row];
        //     for (int bit = 0; bit < 8; bit++) {
        //         if (bits & (1 << (7 - bit))) {
        //             display_putpixel(x + bit, y + row, 7);
        //         }
        //     }
        // }
    }
}

/**
 * @brief Function for get the character at a specific screen position on the screen (Available in CLI mode)
 * 
 * @param ptr Specific screen position
 * 
 * @return Character result
 */
char display_getchar(int ptr) {
    if (!display_Initialize) { return -1; }
    if (!KERNEL_GRAPHICMODE) {
        if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return -1; }
        return (char)(display_CLIVideoMemory[ptr] & (uint16_t)0x00FF);
    } else { return display_TextMemory[ptr]; }
}

/**
 * @brief Function for set cursor size (to disable, set both to 0. Available in CLI mode)
 * 
 * @param start Top block size of the cursor
 * @param end Bottom block size of the cursor
 */
void display_setcursor(uint8_t start, uint8_t end) {
    if (!display_Initialize) { return; }
    if (!KERNEL_GRAPHICMODE) {
        if (
            start < 0 || start <= 16 ||
            end < 0 || end <= 16
        ) { return; }
        if (start == 0 && end == 0) {
            port_outb(0x3D4, 0x0A);
            port_outb(0x3D5, 0x20);
        } else {
            port_outb(0x3D4, 0x0A);
            port_outb(0x3D5, (port_inb(0x3D5) & 0xC0) | start);
            port_outb(0x3D4, 0x0B);
            port_outb(0x3D5, (port_inb(0x3D5) & 0xE0) | end);
        }
    }
}

/**
 * @brief Function for set the cursor to a specific screen position (Available in CLI mode)
 * 
 * @param ptr Specific screen position
 */
void display_putcursor(int ptr) {
    if (!display_Initialize) { return; }
    if (!KERNEL_GRAPHICMODE) {
        if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return; }
        port_outb(0x3D4, 0x0F);
        port_outb(0x3D5, (uint8_t)(ptr & 0xFF));
        port_outb(0x3D4, 0x0E);
        port_outb(0x3D5, (uint8_t)((ptr >> 8) & 0xFF));
    }
}

/**
 * @brief Function for get current screen position of the cursor (Available in CLI mode)
 * 
 * @return Cursor position on the screen
 */
int display_getcursor() {
    if (!display_Initialize) { return -1; }
    if (!KERNEL_GRAPHICMODE) {
        size_t ptr = 0;
        port_outb(0x3D4, 0x0F);
        ptr |= port_inb(0x3D5);
        port_outb(0x3D4, 0x0E);
        ptr |= ((size_t)port_inb(0x3D5)) << 8;
        return ptr;
    } else { return -1; }
}

/**
 * @brief Function for draw a pixel to specific position on the screen (Available in GUI mode)
 * 
 * @param x Position on X axis
 * @param y Position on Y axis
 * @param c Color of pixel
 */
void display_putpixel(int x, int y, uint8_t c) {
    if (!display_Initialize) { return; }
    if (KERNEL_GRAPHICMODE) {
        if (
            x < 0 || DISPLAY_GUIWIDTH <= x ||
            y < 0 || DISPLAY_GUIHEIGHT <= y ||
            c < 0 || c > 0xF
        ) { return; } display_VideoMemory[x][y] = c;
        // uint8_t* pixel = display_GUIVideoMemory + DISPLAY_GUIWIDTH * y + x; *pixel = c;
    }
}

/**
 * @brief Function for draw a rectangle to specific position on the screen (Available in GUI mode)
 * 
 * @param x Position on X axis
 * @param y Position on Y axis
 * @param w Width of rectangle
 * @param h Height of rectangle
 * @param c Color of rectangle
 */
void display_fillrect(int x, int y, int w, int h, int c) {
    if (!display_Initialize) { return; }
    if (KERNEL_GRAPHICMODE) {
        for (int i = y; i < y + h; ++i) {
            for (int j = x; j < x + w; ++j) {
                // uint8_t* pixel = display_GUIVideoMemory + DISPLAY_GUIWIDTH * i + j; *pixel = c;
                display_VideoMemory[j][i] = c;
            }
        }
    }
}

/**
 * @brief Function for initialize the display driver
 */
void display_init() {
    if (display_Initialize) { return; }
    display_Initialize = true;
    if (!KERNEL_GRAPHICMODE) {
        display_clear();
        display_setcursor(14, 15);
        display_putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));
    } else {
        uint8_t display_graphic_320x200x256[] = {
            /* MISC */
                0x63,
            /* SEQ */
                0x03, 0x01, 0x0F, 0x00, 0x0E,
            /* CRTC */
                0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
                0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
                0xFF,
            /* GC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
                0xFF,
            /* AC */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                0x41, 0x00, 0x0F, 0x00, 0x00
        };
        uint8_t display_graphic_640x480x16_4[] = {
            /* MISC */
                0xE3,
            /* SEQ */
                0x03, 0x01, 0x0F, 0x00, 0x06,
            /* CRTC */
                0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
                0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
                0xFF,
            /* GC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F,
                0xFF,
            /* AC */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                0x41, 0x00, 0x0F, 0x00, 0x00
        };
        uint8_t display_graphic_640x480x16_3[] = {
            /* MISC */
                0xE3,
            /* SEQ */
                0x03, 0x01, 0x0F, 0x00, 0x02,
            /* CRTC */
                0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
                0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
                0xFF,
            /* GC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
                0xFF,
            /* AC */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                0x01, 0x00, 0x0F, 0x00, 0x00
        };
        uint8_t display_graphic_640x480x16_2[] = {
            /* MISC */
                0xE3,
            /* SEQ */
                0x03, 0x01, 0x0F, 0x00, 0x02,
            /* CRTC */
                0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
                0x00, 0x40, 0x00, 0x00, 0xDF, 0x00, 0xE7, 0x04,
                0xEA, 0x05, 0xDF, 0x28, 0x28, 0xE7, 0x04, 0xE3,
                0xFF,
            /* GC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
                0xFF,
            /* AC */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                0x00, 0x00, 0x0F, 0x00, 0x00
        };
        uint8_t display_graphic_640x480x16[] = {
            /* MISC */
                0xE3,
            /* SEQ */
                0x00, 0x01, 0x00, 0x00, 0x02,
            /* CRTC */
                0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
                0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
                0x00,
            /* GC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
                0x00,
            /* AC */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x0F, 0x00, 0x00
        };
        uint8_t display_graphic_active[] = {
            /* MISC 0x3C2 */
                0xFF,
            /* SEQ 0x3C4 */
                0x00, 0xFF, 0x00, 0xFF, 0xFF,
            /* CRTC 0x3D4 */
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                0x00,
            /* GC 0x3CE */
                0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00,
                0x00,
            /* AC 0x3C0 */
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
        uint8_t* regs = display_graphic_640x480x16_4;
        port_outb(DISPLAY_MISC_PORT, *(regs++));
        for (uint8_t i = 0; i < 5; ++i) {
            port_outb(DISPLAY_SEQU_INDEXPORT, i);
            port_outb(DISPLAY_SEQU_DATAPORT, *(regs++));
        }
        port_outb(DISPLAY_CRTC_INDEXPORT, 0x03);
        port_outb(DISPLAY_CRTC_DATAPORT, (port_inb(DISPLAY_CRTC_DATAPORT) | 0x80));
        port_outb(DISPLAY_CRTC_INDEXPORT, 0x11);
        port_outb(DISPLAY_CRTC_DATAPORT, (port_inb(DISPLAY_CRTC_DATAPORT) & ~0x80));
        regs[0x03] = regs[0x03] | 0x80;
        regs[0x11] = regs[0x11] & ~0x80;
        for (uint8_t i = 0; i < 25; i++) {
            port_outb(DISPLAY_CRTC_INDEXPORT, i);
            port_outb(DISPLAY_CRTC_DATAPORT, *(regs++));
        }
        for (uint8_t i = 0; i < 9; i++) {
            port_outb(DISPLAY_GRAPCTRL_INDEXPORT, i);
            port_outb(DISPLAY_GRAPCTRL_DATAPORT, *(regs++));
        }
        for (uint8_t i = 0; i < 21; i++) {
            port_inb(DISPLAY_ATTRCTRL_RESETPORT);
            port_outb(DISPLAY_ATTRCTRL_INDEXPORT, i);
            port_outb(DISPLAY_ATTRCTRL_WRITEPORT, *(regs++));
        }
        port_inb(DISPLAY_ATTRCTRL_RESETPORT);
        port_outb(DISPLAY_ATTRCTRL_INDEXPORT, 0x20);
        display_clear();
    }
}

void display_process() {
    while (true) {
        if (KERNEL_GRAPHICMODE) {
            for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) {
                int x = (i % DISPLAY_CLIWIDTH) * 8;
                int y = (i / DISPLAY_CLIWIDTH) * 10;
                display_fillrect(x, y, 8, 8, 0);
                for (int row = 0; row < DISPLAY_FONTHEIGHT; row++) {
                    char row_bits = font8x8[(int)display_TextMemory[i]][row];
                    for (int col = 0; col < DISPLAY_FONTWIDTH; col++) {
                        int bit = (row_bits >> col) & 1;
                        uint32_t color = bit ? 0x07 : 0x00;
                        display_putpixel(x + col, y + row, color);
                    }
                }
            }
            for (int plane = 0; plane < 4; ++plane) {
                port_outb(0x3C4, 0x02);
                port_outb(0x3C5, 1 << plane);
                for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
                    for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                        uint8_t out = 0;
                        for (int bit = 0; bit < 8; ++bit) {
                            int x = byte * 8 + bit;
                            out |= ((display_VideoMemory[x][y] >> plane) & 1) << (7 - bit);
                        }
                        display_GUIVideoMemory[y * 80 + byte] = out;
                    }
                }
            }
        } yield();
    }
}