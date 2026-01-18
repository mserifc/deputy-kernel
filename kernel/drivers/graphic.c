#include "drivers/display.h"

#define DISPLAY_GRAPHIC_WIDTH 320
#define DISPLAY_GRAPHIC_HEIGHT 200

#define DISPLAY_GRAPHIC_MISC_PORT 0x3c2
#define DISPLAY_GRAPHIC_CRTC_INDEXPORT 0x3d4
#define DISPLAY_GRAPHIC_CRTC_DATAPORT 0x3d5
#define DISPLAY_GRAPHIC_SEQUENCER_INDEXPORT 0x3c4
#define DISPLAY_GRAPHIC_SEQUENCER_DATAPORT 0x3c5
#define DISPLAY_GRAPHIC_GRAPHICSCONTROLLER_INDEXPORT 0x3ce
#define DISPLAY_GRAPHIC_GRAPHICSCONTROLLER_DATAPORT 0x3cf
#define DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_INDEXPORT 0x3c0
#define DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_READPORT 0x3c1
#define DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_WRITEPORT 0x3c0
#define DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_RESETPORT 0x3da

void display_graphic_writeRegisters(uint8_t* regs) {
    port_outb(DISPLAY_GRAPHIC_MISC_PORT, *(regs++));
    for (uint8_t i = 0; i < 5; ++i) {
        port_outb(DISPLAY_GRAPHIC_SEQUENCER_INDEXPORT, i);
        port_outb(DISPLAY_GRAPHIC_SEQUENCER_DATAPORT, *(regs++));
    }
    port_outb(DISPLAY_GRAPHIC_CRTC_INDEXPORT, 0x03);
    port_outb(DISPLAY_GRAPHIC_CRTC_DATAPORT, (port_inb(DISPLAY_GRAPHIC_CRTC_DATAPORT) | 0x80));
    port_outb(DISPLAY_GRAPHIC_CRTC_INDEXPORT, 0x11);
    port_outb(DISPLAY_GRAPHIC_CRTC_DATAPORT, (port_inb(DISPLAY_GRAPHIC_CRTC_DATAPORT) & ~0x80));
    regs[0x03] = regs[0x03] | 0x80;
    regs[0x11] = regs[0x11] & ~0x80;
    for (uint8_t i = 0; i < 25; i++) {
        port_outb(DISPLAY_GRAPHIC_CRTC_INDEXPORT, i);
        port_outb(DISPLAY_GRAPHIC_CRTC_DATAPORT, *(regs++));
    }
    for (uint8_t i = 0; i < 9; i++) {
        port_outb(DISPLAY_GRAPHIC_GRAPHICSCONTROLLER_INDEXPORT, i);
        port_outb(DISPLAY_GRAPHIC_GRAPHICSCONTROLLER_DATAPORT, *(regs++));
    }
    for (uint8_t i = 0; i < 21; i++) {
        port_inb(DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_RESETPORT);
        port_outb(DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_INDEXPORT, i);
        port_outb(DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_WRITEPORT, *(regs++));
    }
    port_inb(DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_RESETPORT);
    port_outb(DISPLAY_GRAPHIC_ATTRIBUTECONTROLLER_INDEXPORT, 0x20);
}

void display_graphic_switch() {
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
    display_graphic_writeRegisters(display_graphic_320x200x256);
    for (int i = 0; i < DISPLAY_GRAPHIC_HEIGHT; ++i) {
        for (int j = 0; j < DISPLAY_GRAPHIC_WIDTH; ++j) {
            uint8_t* pixel = display_graphic_getFrameBufferSegment() + DISPLAY_GRAPHIC_WIDTH * i + j; *pixel = 0x00;
        }
    }
}

uint8_t* display_graphic_getFrameBufferSegment() {
    port_outb(DISPLAY_GRAPHIC_GRAPHICSCONTROLLER_INDEXPORT, 0x06);
    uint8_t segment = port_inb(DISPLAY_GRAPHIC_GRAPHICSCONTROLLER_DATAPORT) & (3<<2);
    switch(segment) {
        default:
        case 0<<2: return (uint8_t*)0x00000;
        case 1<<2: return (uint8_t*)0xA0000;
        case 2<<2: return (uint8_t*)0xB0000;
        case 3<<2: return (uint8_t*)0xB8000;
    }
}

void display_graphic_putPixel(uint32_t x, uint32_t y, uint8_t c) {
    if (
        x < 0 || DISPLAY_GRAPHIC_WIDTH <= x ||
        y < 0 || DISPLAY_GRAPHIC_HEIGHT <= y
    ) { return; }
    uint8_t* pixel = display_graphic_getFrameBufferSegment() + DISPLAY_GRAPHIC_WIDTH * y + x;
    *pixel = c;
}

void display_graphic_fillRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t c) {
    for (int i = y; i < y + h; ++i) {
        for (int j = x; j < x + w; ++j) {
            display_graphic_putPixel(j, i, c);
        }
    }
}

#include "common.h"

uint8_t display_graphic_24to8(uint8_t r, uint8_t g, uint8_t b) {
    // if (r == 0x00 && g == 0x00 && b == 0x00) { return 0x00; }
    // if (r == 0x00 && g == 0x00 && b == 0xFF) { return 0x01; }
    // if (r == 0x00 && g == 0xFF && b == 0x00) { return 0x02; }
    // if (r == 0xFF && g == 0x00 && b == 0x00) { return 0x04; }
    // if (r == 0xFF && g == 0xFF && b == 0xFF) { return 0x3F; }
    // return 0x20;
    bool
        rupper = false, rlower = false,
        gupper = false, glower = false,
        bupper = false, blower = false;
    uint8_t result;
    if (r > 170) {
        rupper = true;
        rlower = true;
    } else if (r > 85) {
        rupper = true;
    } else if (r > 0) {
        rlower = true;
    }
    if (g > 170) {
        gupper = true;
        glower = true;
    } else if (g > 85) {
        gupper = true;
    } else if (g > 0) {
        glower = true;
    }
    if (b > 170) {
        bupper = true;
        blower = true;
    } else if (b > 85) {
        bupper = true;
    } else if (b > 0) {
        blower = true;
    }
    result = (rlower << 5) | (glower << 4) | (blower << 3) | (rupper << 2) | (gupper << 1) | (bupper << 0);
    return result;
}

void display_graphic_bmpViewer(void* image) {
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
            display_graphic_putPixel(j, i, display_graphic_24to8(image_px[n].r, image_px[n].g, image_px[n].b)); n++;
        }
    }
}