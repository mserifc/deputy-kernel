#include "drivers/display.h"

// Pointer for video memory
uint16_t* display_VideoMemory = (uint16_t*)0xB8000;

display_color_t display_BackgroundColor;
display_color_t display_TextColor;

// Function for clear the entire screen
void display_clear() {
    for (int i = 0; i < (DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT); ++i) {
        display_VideoMemory[i] =
            (short)(((display_BackgroundColor << 4) | display_TextColor) << 8) | (short)0x0000;
    }
}

// Function for print a single character at a specified position (pointer) on the screen
void display_putchar(char chr, int ptr) {
    if (ptr >= 0 && ptr < (DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT)) {
        display_VideoMemory[ptr] =
            (short)(((display_BackgroundColor << 4) | display_TextColor) << 8) | (short)chr;
    }
}

// Function for get the character at a specific screen position (pointer)
char display_getchar(int ptr) {
    return (char)(display_VideoMemory[ptr] & (short)0x00FF);
}

// Function for enable the cursor, specifying the cursor size (not position)
void display_enablecursor(uint8_t start, uint8_t end) {
    port_outb(0x3D4, 0x0A);
    port_outb(0x3D5, (port_inb(0x3D5) & 0xC0) | start);
    port_outb(0x3D4, 0x0B);
    port_outb(0x3D5, (port_inb(0x3D5) & 0xE0) | end);
}

// Function for disable the cursor (hide it from the screen)
void display_disablecursor() {
    port_outb(0x3D4, 0x0A);
    port_outb(0x3D5, 0x20);
}

// Function for set the cursor to a specific screen position (pointer)
void display_putcursor(int ptr) {
    port_outb(0x3D4, 0x0F);
    port_outb(0x3D5, (uint8_t)(ptr & 0xFF));
    port_outb(0x3D4, 0x0E);
    port_outb(0x3D5, (uint8_t)((ptr >> 8) & 0xFF));
}

// Function for get the current position of the cursor
size_t display_getcursor() {
    size_t ptr = 0;
    port_outb(0x3D4, 0x0F);
    ptr |= port_inb(0x3D5);
    port_outb(0x3D4, 0x0E);
    ptr |= ((size_t)port_inb(0x3D5)) << 8;
    return ptr;
}

// Function for initialize display driver
void display_init() {
    display_BackgroundColor = (uint8_t)((display_VideoMemory[0] & 0xF000) >> 12);   // Get default display background color
    display_TextColor = (uint8_t)((display_VideoMemory[0] & 0x0F00) >> 8);          // Get default display character color
    display_enablecursor(14, 15);                                                   // Enable display cursor
}