#include "drivers/display_old3.h"
#include "assets/font.h"

// Display driver initialization status
bool display_Initialized = false;

// Current display draw color
uint8_t display_CurrentColor = 0;

// CLI video memory base address
uint16_t* display_CLIVideoMemory = (uint16_t*)DISPLAY_CLIVIDEOMEMORY;

// GUI video memory base address
uint8_t* display_GUIVideoMemory = (uint8_t*)DISPLAY_GUIVIDEOMEMORY;

// CLI display background color
uint8_t display_CLIBackground = DISPLAY_CLIDEFAULT_BACKGROUND;

// CLI display foreground color
uint8_t display_CLIForeground = DISPLAY_CLIDEFAULT_FOREGROUND;

/**
 * @brief Function for clear entire screen (Available in both modes)
 */
void display_clear() {
    if (!display_Initialized) { return; }
    if (!KERNEL_GRAPHICMODE) {
        for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i)
            { display_CLIVideoMemory[i] = (uint16_t)(display_CLIBackground << 4 | display_CLIForeground << 8); }
    } else {
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
 * @brief Function for scroll down the entire screen (Available in GUI mode)
 * 
 * @param lines Count of pixel lines to scroll
 */
void display_scrolldown(int lines) {
    if (!display_Initialized || !KERNEL_GRAPHICMODE) return;
    for (int p = 0; p < DISPLAY_PLANES; ++p) { display_setcolor(p, 0);
        int ln = p ? 0 : lines;
        for (int row = 0; row < DISPLAY_GUIHEIGHT - ln; ++row) {
            for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                display_GUIVideoMemory[row * (DISPLAY_GUIWIDTH / 8) + byte] =
                    display_GUIVideoMemory[(row + ln) * (DISPLAY_GUIWIDTH / 8) + byte];
            }
        }
        for (int row = DISPLAY_GUIHEIGHT - ln; row < DISPLAY_GUIHEIGHT; ++row) {
            for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                display_GUIVideoMemory[row * (DISPLAY_GUIWIDTH / 8) + byte] = 0x00;
            }
        }
    }
}


/**
 * @brief Function for print a single character at a specific position on the screen (Available in both modes)
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
        int x = (ptr % DISPLAY_CLIWIDTH) * FONT_WIDTH;
        int y = (ptr / DISPLAY_CLIWIDTH) * FONT_HEIGHT;
        for (int p = 0; p < DISPLAY_PLANES; ++p) { display_setcolor(p, display_CLIForeground);
            for (int row = 0; row < FONT_HEIGHT; ++row) {
                char row_bits = font8x16[(int)chr][row];
                for (int col = 0; col < FONT_WIDTH; ++col) {
                    int bit = (row_bits >> FONT_WIDTH - 1 - col) & 1;
                    if (bit) { display_putpixel(x + col, y + row); }
                    else { display_rempixel(x + col, y + row); }
                }
            }
        }
    }
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
 * @brief Function for set color for drawing operations
 * 
 * Usage: for (int p = 0; p < DISPLAY_PLANES; ++p) {
 *      You can use drawing operations here after using this function in the loop
 * }
 * 
 * @param s Stage of plane
 * @param c Color
 */
void display_setcolor(int s, uint8_t c) {
    if (!display_Initialized || s < 0 || s > 3) { return; }
    port_outb(0x3C4, 0x02); port_outb(0x3C5, 1 << s);
    uint8_t out = 0; for (int bit = 0; bit < 8; ++bit) { out |= ((c >> s) & 1) << (7 - bit); }
    display_CurrentColor = out;
}

/**
 * @brief Function for draw a pixel to specific position on the screen (Available in GUI mode)
 * 
 * @param x Position on X axis
 * @param y Position on Y axis
 */
void display_putpixel(int x, int y) {
    if (!display_Initialized) { return; }
    if (KERNEL_GRAPHICMODE) {
        if (x < 0 || DISPLAY_GUIWIDTH <= x ||
            y < 0 || DISPLAY_GUIHEIGHT <= y) { return; }
        int byte = y * (DISPLAY_GUIWIDTH / 8) + (x / 8); int bit = x % 8;
        if (display_CurrentColor) { display_GUIVideoMemory[byte] |= (1 << (7 - bit)); }
        else { display_GUIVideoMemory[byte] &= ~(1 << (7 - bit)); }
    }
}

/**
 * @brief Function for remove a pixel (set black) from specific position on the screen (Available in GUI mode)
 * 
 * @param x Position on X axis
 * @param y Position on Y axis
 */
void display_rempixel(int x, int y) {
    if (!display_Initialized) { return; }
    if (KERNEL_GRAPHICMODE) {
        if (x < 0 || DISPLAY_GUIWIDTH <= x ||
            y < 0 || DISPLAY_GUIHEIGHT <= y) { return; }
        int byte = y * (DISPLAY_GUIWIDTH / 8) + (x / 8);
        int bit = x % 8; display_GUIVideoMemory[byte] &= ~(1 << (7 - bit));
    }
}

/**
 * @brief Function for draw a rectangle to specific position on the screen (Available in GUI mode)
 * 
 * @param x Position on X axis
 * @param y Position on Y axis
 * @param w Width of rectangle
 * @param h Height of rectangle
 */
void display_fillrect(int x, int y, int w, int h) {
    if (!display_Initialized) { return; }
    if (KERNEL_GRAPHICMODE) {
        for (int i = y; i < y + h; ++i) {
            for (int j = x; j < x + w; ++j) {
                int byte = i * (DISPLAY_GUIWIDTH / 8) + (j / 8); int bit = j % 8;
                if (display_CurrentColor) { display_GUIVideoMemory[byte] |= (1 << (7 - bit)); }
                else { display_GUIVideoMemory[byte] &= ~(1 << (7 - bit)); }
            }
        }
    }
}

uint8_t display_convert24to4(uint8_t r, uint8_t g, uint8_t b) {
    if (r < 32 && g < 32 && b < 32) return 0x0;  // Black
    if (r > 192 && g < 64 && b < 64) return 0x4; // Red
    if (r < 64 && g > 192 && b < 64) return 0x2; // Green
    if (r < 64 && g < 64 && b > 192) return 0x1; // Blue
    if (r > 192 && g > 192 && b < 64) return 0xE; // Yellow
    if (r > 192 && g < 64 && b > 192) return 0x5; // Magenta
    if (r < 64 && g > 192 && b > 192) return 0x3; // Cyan
    if (r > 128 && g > 128 && b > 128) return 0x7; // Light Gray
    return 0x8; // Dark Gray
}

void display_renderBMP(void* image, int x, int y) {
    struct info {
        uint32_t size;
        int32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bitCount;
        uint32_t compression;
        uint32_t sizeImage;
        int32_t XPelsPerMeter;
        int32_t YPelsPerMeter;
        uint32_t clrUsed;
        uint32_t clrImportant;
    };
    struct pixel {
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
    void* image_information = image + 14;
    void* image_data = image + 54;
    struct info* image_info = image_information;
    struct pixel* image_px = image_data;
    int n = 0;
    for (int i = image_info->height; i > 0; --i) {
        for (int j = 0; j < image_info->width; ++j) {
            for (int p = 0; p < DISPLAY_PLANES; ++p) {
                display_setcolor(p, display_convert24to4(image_px[n].r, image_px[n].g, image_px[n].b));
                display_putpixel(j, i); n++;
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
    } display_Initialized = true;
}