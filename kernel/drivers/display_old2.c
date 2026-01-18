#include "drivers/display_old2.h"
#include "assets/font.h"

// Display driver initialization status
bool display_Initialized = false;

// Display change flag
bool display_Changed = false;

// Current display draw color
uint8_t display_CurrentColor = 0;

// CLI video memory base address
uint16_t* display_CLIVideoMemory = (uint16_t*)DISPLAY_CLIVIDEOMEMORY;

// GUI video memory base address
uint8_t* display_GUIVideoMemory = (uint8_t*)DISPLAY_GUIVIDEOMEMORY;

// Display text memory
char display_TextMemory[DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT];

// Display video memory
uint8_t display_VideoMemory[DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT];

// CLI display background color
color_t display_CLIBackground = DISPLAY_CLIDEFAULT_BACKGROUND;

// CLI display foreground color
color_t display_CLIForeground = DISPLAY_CLIDEFAULT_FOREGROUND;

/**
 * @brief Function for clear entire screen (Available in both modes)
 */
void display_clear() {
    if (!display_Initialized) { return; }
    if (!KERNEL_GRAPHICMODE) {
        for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i)
            { display_CLIVideoMemory[i] = (uint16_t)(display_CLIBackground << 4 | display_CLIForeground << 8); }
    } else {
        for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) { display_TextMemory[i] = 0; }
        for (int plane = 0; plane < 4; ++plane) {
            port_outb(0x3C4, 0x02); port_outb(0x3C5, 1 << plane);
            for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
                for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                    display_GUIVideoMemory[y * 80 + byte] = 0;
                }
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
    if (!display_Initialized) { return; }
    if (!KERNEL_GRAPHICMODE) {
        if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return; }
        display_CLIVideoMemory[ptr] = (uint16_t)(display_CLIBackground << 4 | display_CLIForeground << 8) | chr;
    } else {
        display_TextMemory[ptr] = chr;
        int x = (ptr % DISPLAY_CLIWIDTH) * FONT_WIDTH;
        int y = (ptr / DISPLAY_CLIWIDTH) * FONT_HEIGHT;
        for (int i = 0; i < 4; ++i) {
            display_setplane(i, 0x07);
            // display_fillrect(x, y, FONT_WIDTH, FONT_HEIGHT);
            for (int row = 0; row < FONT_HEIGHT; ++row) {
                char row_bits = font8x16[(int)chr][row];
                for (int col = 0; col < FONT_WIDTH; ++col) {
                    int bit = (row_bits >> FONT_WIDTH - 1 - col) & 1;
                    // uint32_t color = bit ? 0x07 : 0x00;
                    if (bit) { display_putpixel(x + col, y + row); } else { display_rempixel(x + col, y + row); }
                }
            }
        }
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
    if (!display_Initialized) { return 0; }
    if (!KERNEL_GRAPHICMODE) {
        if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return -1; }
        return (char)(display_CLIVideoMemory[ptr] & 0xFF);
    } else { return display_TextMemory[ptr]; }
}

/**
 * @brief Function for set the cursor to a specific screen position (Available in CLI mode)
 * 
 * @param ptr Specific screen position
 */
void display_putcursor(int ptr) {
    if (!display_Initialized) { return; }
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
    if (!display_Initialized) { return 0; }
    if (!KERNEL_GRAPHICMODE) {
        size_t ptr = 0;
        port_outb(0x3D4, 0x0F);
        ptr |= port_inb(0x3D5);
        port_outb(0x3D4, 0x0E);
        ptr |= ((size_t)port_inb(0x3D5)) << 8;
        return ptr;
    } else { return 0; }
}

void display_setplane(int n, uint8_t c) {
    if (!display_Initialized || n < 0 || n > 3) { return; }
    port_outb(0x3C4, 0x02); port_outb(0x3C5, 1 << n);
    uint8_t out = 0; for (int bit = 0; bit < 8; ++bit) { out |= ((c >> n) & 1) << (7 - bit); }
    display_CurrentColor = out;
}

/**
 * @brief Function for draw a pixel to specific position on the screen (Available in GUI mode)
 * 
 * @param x Position on X axis
 * @param y Position on Y axis
 * @param c Color of pixel
 */
void display_putpixel(int x, int y) {
    if (!display_Initialized) { return; }
    if (KERNEL_GRAPHICMODE) {
        if (x < 0 || DISPLAY_GUIWIDTH <= x ||
            y < 0 || DISPLAY_GUIHEIGHT <= y) { return; }
        int byteIndex = y * (DISPLAY_GUIWIDTH / 8) + (x / 8);
        int bitIndex = x % 8;
        if (display_CurrentColor) { display_GUIVideoMemory[byteIndex] |= (1 << (7 - bitIndex)); }
        else { display_GUIVideoMemory[byteIndex] &= ~(1 << (7 - bitIndex)); }
    }
}

void display_rempixel(int x, int y) {
    if (!display_Initialized) { return; }
    if (KERNEL_GRAPHICMODE) {
        if (x < 0 || DISPLAY_GUIWIDTH <= x ||
            y < 0 || DISPLAY_GUIHEIGHT <= y) { return; }
        int byteIndex = y * (DISPLAY_GUIWIDTH / 8) + (x / 8);
        int bitIndex = x % 8; display_GUIVideoMemory[byteIndex] &= ~(1 << (7 - bitIndex));
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
void display_fillrect(int x, int y, int w, int h) {
    if (!display_Initialized) { return; }
    if (KERNEL_GRAPHICMODE) {
        for (int i = y; i < y + h; ++i) {
            for (int j = x; j < x + w; ++j) {
                int byteIndex = y * (DISPLAY_GUIWIDTH / 8) + (x / 8);
                int bitIndex = x % 8;
                if (display_CurrentColor) { display_GUIVideoMemory[byteIndex] |= (1 << (7 - bitIndex)); }
                else { display_GUIVideoMemory[byteIndex] &= ~(1 << (7 - bitIndex)); }
            }
        }
    }
}

/**
 * @brief Function for initialize the display driver
 */
void display_init() {
    if (display_Initialized) { return; }
    if (!KERNEL_GRAPHICMODE) {
        display_clear();
        port_outb(0x3D4, 0x0A);
        port_outb(0x3D5, (port_inb(0x3D5) & 0xC0) | 14);
        port_outb(0x3D4, 0x0B);
        port_outb(0x3D5, (port_inb(0x3D5) & 0xE0) | 15);
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
        uint8_t display_graphic_640x480x16[] = {
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
        uint8_t* regs = display_graphic_640x480x16;
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
        for (int plane = 0; plane < 4; ++plane) {
            port_outb(0x3C4, 0x02); port_outb(0x3C5, 1 << plane);
            for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
                for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                    display_GUIVideoMemory[y * 80 + byte] = 0;
                }
            }
        }
        for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) { display_TextMemory[i] = 0; }
        // for (int i = 0; i < DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT; ++i) { display_VideoMemory[i] = 0; }
        display_Changed = true;
    } display_Initialized = true;
}

/**
 * @brief Main function of VGA display driver process for keep display up to date
 */
void display_process() {
    if (1) { exit(); return; }
    if (!display_Initialized || !KERNEL_GRAPHICMODE) { exit(); return; }
    while (true) {
        if (!display_Changed) { yield(); continue; }
        for (int plane = 0; plane < 4; ++plane) {
            port_outb(0x3C4, 0x02);
            port_outb(0x3C5, 1 << plane);
            for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
                for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                    uint8_t out = 0;
                    for (int bit = 0; bit < 8; ++bit) {
                        int x = byte * 8 + bit;
                        out |= ((display_VideoMemory[y * DISPLAY_GUIWIDTH + x] >> plane) & 1) << (7 - bit);
                    } display_GUIVideoMemory[y * (DISPLAY_GUIWIDTH / 8) + byte] = out;
                }
            }
        } yield();
    }
}

// #include "drivers/display.h"
// #include "assets/font.h"

// // Display driver initialization status
// bool display_Initialized = false;

// // Display change flag
// bool display_Changed = false;

// // Current display draw color
// uint8_t display_CurrentColor = 0;

// // CLI video memory base address
// uint16_t* display_CLIVideoMemory = (uint16_t*)DISPLAY_CLIVIDEOMEMORY;

// // GUI video memory base address
// uint8_t* display_GUIVideoMemory = (uint8_t*)DISPLAY_GUIVIDEOMEMORY;

// // Display text memory
// char display_TextMemory[DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT];

// // Display video memory
// uint8_t display_VideoMemory[DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT];

// // CLI display background color
// color_t display_CLIBackground = DISPLAY_CLIDEFAULT_BACKGROUND;

// // CLI display foreground color
// color_t display_CLIForeground = DISPLAY_CLIDEFAULT_FOREGROUND;

// /**
//  * @brief Function for clear entire screen (Available in both modes)
//  */
// void display_clear() {
//     if (!display_Initialized) { return; }
//     if (!KERNEL_GRAPHICMODE) {
//         for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i)
//             { display_CLIVideoMemory[i] = (uint16_t)(display_CLIBackground << 4 | display_CLIForeground << 8); }
//     } else {
//         for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) { display_TextMemory[i] = 0; }
//         for (int i = 0; i < DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT; ++i) { display_VideoMemory[i] = 0; }
//         display_Changed = true;
//     }
// }

// /**
//  * @brief Function for print a single character at a specific position on the screen (Available in CLI mode)
//  * 
//  * @param chr Character to be print
//  * @param ptr Specific screen position
//  */
// void display_putchar(char chr, int ptr) {
//     if (!display_Initialized) { return; }
//     if (!KERNEL_GRAPHICMODE) {
//         if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return; }
//         display_CLIVideoMemory[ptr] = (uint16_t)(display_CLIBackground << 4 | display_CLIForeground << 8) | chr;
//     } else {
//         display_TextMemory[ptr] = chr;
//         int x = (ptr % DISPLAY_CLIWIDTH) * FONT_WIDTH;
//         int y = (ptr / DISPLAY_CLIWIDTH) * FONT_HEIGHT;
//         for (int row = 0; row < FONT_HEIGHT; ++row) {
//             char row_bits = font8x16[(int)chr][row];
//             for (int col = 0; col < FONT_WIDTH; ++col) {
//                 int bit = (row_bits >> FONT_WIDTH - 1 - col) & 1;
//                 uint32_t color = bit ? 0x07 : 0x00;
//                 display_putpixel(x + col, y + row, color);
//             }
//         }
//     }
// }

// /**
//  * @brief Function for get the character at a specific screen position on the screen (Available in CLI mode)
//  * 
//  * @param ptr Specific screen position
//  * 
//  * @return Character result
//  */
// char display_getchar(int ptr) {
//     if (!display_Initialized) { return 0; }
//     if (!KERNEL_GRAPHICMODE) {
//         if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return -1; }
//         return (char)(display_CLIVideoMemory[ptr] & 0xFF);
//     } else { return display_TextMemory[ptr]; }
// }

// /**
//  * @brief Function for set the cursor to a specific screen position (Available in CLI mode)
//  * 
//  * @param ptr Specific screen position
//  */
// void display_putcursor(int ptr) {
//     if (!display_Initialized) { return; }
//     if (!KERNEL_GRAPHICMODE) {
//         if (ptr < 0 || DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT <= ptr) { return; }
//         port_outb(0x3D4, 0x0F);
//         port_outb(0x3D5, (uint8_t)(ptr & 0xFF));
//         port_outb(0x3D4, 0x0E);
//         port_outb(0x3D5, (uint8_t)((ptr >> 8) & 0xFF));
//     }
// }

// /**
//  * @brief Function for get current screen position of the cursor (Available in CLI mode)
//  * 
//  * @return Cursor position on the screen
//  */
// int display_getcursor() {
//     if (!display_Initialized) { return 0; }
//     if (!KERNEL_GRAPHICMODE) {
//         size_t ptr = 0;
//         port_outb(0x3D4, 0x0F);
//         ptr |= port_inb(0x3D5);
//         port_outb(0x3D4, 0x0E);
//         ptr |= ((size_t)port_inb(0x3D5)) << 8;
//         return ptr;
//     } else { return 0; }
// }

// void display_setplane(int n, uint8_t c) {
//     if (!display_Initialized || n < 0 || n > 3) { return; }
//     port_outb(0x3C4, 0x02); port_outb(0x3C5, 1 << n);
//     uint8_t out = 0; for (int bit = 0; bit < 8; ++bit) { out |= ((c >> n) & 1) << (7 - bit); }
//     display_CurrentColor = out;
// }

// /**
//  * @brief Function for draw a pixel to specific position on the screen (Available in GUI mode)
//  * 
//  * @param x Position on X axis
//  * @param y Position on Y axis
//  * @param c Color of pixel
//  */
// void display_putpixel(int x, int y, uint8_t c) {
//     if (!display_Initialized) { return; }
//     if (KERNEL_GRAPHICMODE) {
//         if (x < 0 || DISPLAY_GUIWIDTH <= x ||
//             y < 0 || DISPLAY_GUIHEIGHT <= y ||
//             c < 0 || c > 0xF) { return; }
//         display_VideoMemory[y * DISPLAY_GUIWIDTH + x] = c; display_Changed = true;
//     }
// }

// /**
//  * @brief Function for draw a rectangle to specific position on the screen (Available in GUI mode)
//  * 
//  * @param x Position on X axis
//  * @param y Position on Y axis
//  * @param w Width of rectangle
//  * @param h Height of rectangle
//  * @param c Color of rectangle
//  */
// void display_fillrect(int x, int y, int w, int h, uint8_t c) {
//     if (!display_Initialized) { return; }
//     if (KERNEL_GRAPHICMODE) {
//         for (int i = y; i < y + h; ++i) {
//             for (int j = x; j < x + w; ++j) {
//                 display_VideoMemory[i * DISPLAY_GUIWIDTH + j] = c;
//                 display_Changed = true;
//             }
//         }
//     }
// }

// /**
//  * @brief Function for initialize the display driver
//  */
// void display_init() {
//     if (display_Initialized) { return; }
//     if (!KERNEL_GRAPHICMODE) {
//         display_clear();
//         port_outb(0x3D4, 0x0A);
//         port_outb(0x3D5, (port_inb(0x3D5) & 0xC0) | 14);
//         port_outb(0x3D4, 0x0B);
//         port_outb(0x3D5, (port_inb(0x3D5) & 0xE0) | 15);
//         display_putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));
//     } else {
//         uint8_t display_graphic_320x200x256[] = {
//             /* MISC */
//                 0x63,
//             /* SEQ */
//                 0x03, 0x01, 0x0F, 0x00, 0x0E,
//             /* CRTC */
//                 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
//                 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
//                 0xFF,
//             /* GC */
//                 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
//                 0xFF,
//             /* AC */
//                 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//                 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
//                 0x41, 0x00, 0x0F, 0x00, 0x00
//         };
//         uint8_t display_graphic_640x480x16[] = {
//             /* MISC */
//                 0xE3,
//             /* SEQ */
//                 0x03, 0x01, 0x0F, 0x00, 0x06,
//             /* CRTC */
//                 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
//                 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                 0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
//                 0xFF,
//             /* GC */
//                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F,
//                 0xFF,
//             /* AC */
//                 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//                 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
//                 0x41, 0x00, 0x0F, 0x00, 0x00
//         };
//         uint8_t* regs = display_graphic_640x480x16;
//         port_outb(DISPLAY_MISC_PORT, *(regs++));
//         for (uint8_t i = 0; i < 5; ++i) {
//             port_outb(DISPLAY_SEQU_INDEXPORT, i);
//             port_outb(DISPLAY_SEQU_DATAPORT, *(regs++));
//         }
//         port_outb(DISPLAY_CRTC_INDEXPORT, 0x03);
//         port_outb(DISPLAY_CRTC_DATAPORT, (port_inb(DISPLAY_CRTC_DATAPORT) | 0x80));
//         port_outb(DISPLAY_CRTC_INDEXPORT, 0x11);
//         port_outb(DISPLAY_CRTC_DATAPORT, (port_inb(DISPLAY_CRTC_DATAPORT) & ~0x80));
//         regs[0x03] = regs[0x03] | 0x80;
//         regs[0x11] = regs[0x11] & ~0x80;
//         for (uint8_t i = 0; i < 25; i++) {
//             port_outb(DISPLAY_CRTC_INDEXPORT, i);
//             port_outb(DISPLAY_CRTC_DATAPORT, *(regs++));
//         }
//         for (uint8_t i = 0; i < 9; i++) {
//             port_outb(DISPLAY_GRAPCTRL_INDEXPORT, i);
//             port_outb(DISPLAY_GRAPCTRL_DATAPORT, *(regs++));
//         }
//         for (uint8_t i = 0; i < 21; i++) {
//             port_inb(DISPLAY_ATTRCTRL_RESETPORT);
//             port_outb(DISPLAY_ATTRCTRL_INDEXPORT, i);
//             port_outb(DISPLAY_ATTRCTRL_WRITEPORT, *(regs++));
//         }
//         port_inb(DISPLAY_ATTRCTRL_RESETPORT);
//         port_outb(DISPLAY_ATTRCTRL_INDEXPORT, 0x20);
//         for (int plane = 0; plane < 4; ++plane) {
//             port_outb(0x3C4, 0x02); port_outb(0x3C5, 1 << plane);
//             for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
//                 for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
//                     display_GUIVideoMemory[y * 80 + byte] = 0;
//                 }
//             }
//         }
//         for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) { display_TextMemory[i] = 0; }
//         for (int i = 0; i < DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT; ++i) { display_VideoMemory[i] = 0; }
//         display_Changed = true;
//     } display_Initialized = true;
// }

// /**
//  * @brief Main function of VGA display driver process for keep display up to date
//  */
// void display_process() {
//     if (!display_Initialized || !KERNEL_GRAPHICMODE) { exit(); return; }
//     while (true) {
//         if (!display_Changed) { yield(); continue; }
//         for (int plane = 0; plane < 4; ++plane) {
//             port_outb(0x3C4, 0x02);
//             port_outb(0x3C5, 1 << plane);
//             for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
//                 for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
//                     uint8_t out = 0;
//                     for (int bit = 0; bit < 8; ++bit) {
//                         int x = byte * 8 + bit;
//                         out |= ((display_VideoMemory[y * DISPLAY_GUIWIDTH + x] >> plane) & 1) << (7 - bit);
//                     } display_GUIVideoMemory[y * (DISPLAY_GUIWIDTH / 8) + byte] = out;
//                 }
//             }
//         } yield();
//     }
// }