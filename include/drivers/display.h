#pragma once

#include "types.h"
#include "platform/i386/port.h"

// Define screen width and height for the Command Line Interface (CLI)
#define display_CLIWIDTH 80     // 80 columns
#define display_CLIHEIGHT 25    // 25 rows

// Enumeration for display color
enum display_COLORTABLE {
    display_COLOR_BLACK         = 0x00,
    display_COLOR_BLUE          = 0x01,
    display_COLOR_GREEN         = 0x02,
    display_COLOR_CYAN          = 0x03,
    display_COLOR_RED           = 0x04,
    display_COLOR_MAGENTA       = 0x05,
    display_COLOR_BROWN         = 0x06,
    display_COLOR_GRAY          = 0x07,
    display_COLOR_DARK_GRAY     = 0x08,
    display_COLOR_LIGHT_BLUE    = 0x09,
    display_COLOR_LIGHT_GREEN   = 0x0A,
    display_COLOR_LIGHT_CYAN    = 0x0B,
    display_COLOR_LIGHT_RED     = 0x0C,
    display_COLOR_LIGHT_MAGENTA = 0x0D,
    display_COLOR_YELLOW        = 0x0E,
    display_COLOR_WHITE         = 0x0F
};

// Enum type alias for display color, to use as a color reference
typedef enum display_COLORTABLE display_color_t;

// Function prototypes
void display_clear();                                   // Clear the entire screen
void display_putchar(char chr, int ptr);                // Print a single character at a specified position (pointer) on the screen
char display_getchar(int ptr);                          // Get the character at a specific screen position (pointer)
void display_enablecursor(uint8_t start, uint8_t end);  // Enable the cursor, specifying the cursor size (not position)
void display_disablecursor();                           // Disable the cursor (hide it from the screen)
void display_putcursor(int ptr);                        // Set the cursor to a specific screen position (pointer)
size_t display_getcursor();                             // Get the current position of the cursor
void display_init();                                    // Initialize display driver