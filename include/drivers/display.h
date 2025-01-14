#pragma once

#include "types.h"
#include "platform/i386/port.h"

// Define screen width and height for the Command Line Interface (CLI)
#define DISPLAY_CLIWIDTH 80     // 80 columns
#define DISPLAY_CLIHEIGHT 25    // 25 rows

// Enumeration for display color
enum DISPLAY_COLORTABLE {
    DISPLAY_COLOR_BLACK         = 0x00,
    DISPLAY_COLOR_BLUE          = 0x01,
    DISPLAY_COLOR_GREEN         = 0x02,
    DISPLAY_COLOR_CYAN          = 0x03,
    DISPLAY_COLOR_RED           = 0x04,
    DISPLAY_COLOR_MAGENTA       = 0x05,
    DISPLAY_COLOR_BROWN         = 0x06,
    DISPLAY_COLOR_GRAY          = 0x07,
    DISPLAY_COLOR_DARK_GRAY     = 0x08,
    DISPLAY_COLOR_LIGHT_BLUE    = 0x09,
    DISPLAY_COLOR_LIGHT_GREEN   = 0x0A,
    DISPLAY_COLOR_LIGHT_CYAN    = 0x0B,
    DISPLAY_COLOR_LIGHT_RED     = 0x0C,
    DISPLAY_COLOR_LIGHT_MAGENTA = 0x0D,
    DISPLAY_COLOR_YELLOW        = 0x0E,
    DISPLAY_COLOR_WHITE         = 0x0F
};

// Define display color type
typedef enum DISPLAY_COLORTABLE display_color_t;

// Function prototypes
void display_clear();                                   // Clear the entire screen
void display_putchar(char chr, int ptr);                // Print a single character at a specified position (pointer) on the screen
char display_getchar(int ptr);                          // Get the character at a specific screen position (pointer)
void display_enablecursor(uint8_t start, uint8_t end);  // Enable the cursor, specifying the cursor size (not position)
void display_disablecursor();                           // Disable the cursor (hide it from the screen)
void display_putcursor(int ptr);                        // Set the cursor to a specific screen position (pointer)
size_t display_getcursor();                             // Get the current position of the cursor
void display_init();                                    // Initialize display driver