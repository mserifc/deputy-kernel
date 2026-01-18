#pragma once

#include "kernel.h"
#include "types.h"
#include "hardware/port.h"

// * Types

typedef unsigned char color_t;      // Color type for represent color values

// * Constants

#define DISPLAY_CLIVIDEOMEMORY          0xB8000 // CLI video memory base address
#define DISPLAY_GUIVIDEOMEMORY          0xA0000 // GUI video memory base address

#define DISPLAY_CLIWIDTH                80      // CLI display width
#define DISPLAY_CLIHEIGHT               25      // CLI display height

#define DISPLAY_GUIWIDTH                320     // GUI display width
#define DISPLAY_GUIHEIGHT               200     // GUI display height

#define DISPLAY_FONTWIDTH               8       // GUI display font width
#define DISPLAY_FONTHEIGHT              8       // GUI display font height

#define DISPLAY_MISC_PORT               0x3c2   // Miscellaneous output port
#define DISPLAY_CRTC_INDEXPORT          0x3d4   // CRTC (Cathode Ray Tube Controller) index port
#define DISPLAY_CRTC_DATAPORT           0x3d5   // CRTC (Cathode Ray Tube Controller) data port
#define DISPLAY_SEQU_INDEXPORT          0x3c4   // Sequencer index port
#define DISPLAY_SEQU_DATAPORT           0x3c5   // Sequencer data port
#define DISPLAY_GRAPCTRL_INDEXPORT      0x3ce   // Graphics controller index port
#define DISPLAY_GRAPCTRL_DATAPORT       0x3cf   // Graphics controller data port
#define DISPLAY_ATTRCTRL_INDEXPORT      0x3c0   // Attribute controller index port
#define DISPLAY_ATTRCTRL_READPORT       0x3c1   // Attribute controller read port
#define DISPLAY_ATTRCTRL_WRITEPORT      0x3c0   // Attribute controller write port
#define DISPLAY_ATTRCTRL_RESETPORT      0x3da   // Attribute controller reset port

// * CLI color values

#define DISPLAY_CLICOLOR_BLACK          0x00    // Black color for CLI display
#define DISPLAY_CLICOLOR_BLUE           0x01    // Blue color for CLI display
#define DISPLAY_CLICOLOR_GREEN          0x02    // Green color for CLI display
#define DISPLAY_CLICOLOR_CYAN           0x03    // Cyan color for CLI display
#define DISPLAY_CLICOLOR_RED            0x04    // Red color for CLI display
#define DISPLAY_CLICOLOR_MAGENTA        0x05    // Magenta color for CLI display
#define DISPLAY_CLICOLOR_BROWN          0x06    // Brown color for CLI display
#define DISPLAY_CLICOLOR_LIGHTGRAY      0x07    // Light gray color for CLI display
#define DISPLAY_CLICOLOR_DARKGRAY       0x08    // Dark gray color for CLI display
#define DISPLAY_CLICOLOR_LIGHTBLUE      0x09    // Light blue color for CLI display
#define DISPLAY_CLICOLOR_LIGHTGREEN     0x0A    // Light green color for CLI display
#define DISPLAY_CLICOLOR_LIGHTCYAN      0x0B    // Light cyan color for CLI display
#define DISPLAY_CLICOLOR_LIGHTRED       0x0C    // Light red color for CLI display
#define DISPLAY_CLICOLOR_PINK           0x0D    // Pink color for CLI display
#define DISPLAY_CLICOLOR_YELLOW         0x0E    // Yellow color for CLI display
#define DISPLAY_CLICOLOR_WHITE          0x0F    // White color for CLI display

// * Defaults

// Default background color for CLI display
#define DISPLAY_CLIDEFAULT_BACKCOLOR    DISPLAY_CLICOLOR_BLACK
// Default text color for CLI display
#define DISPLAY_CLIDEFAULT_TEXTCOLOR    DISPLAY_CLICOLOR_LIGHTGRAY

// * Public functions

void    display_clear();                                        // Clear the entire screen (CLI/GUI)

void    display_putchar(char chr, int ptr);                     // Print a single character at a specific position (CLI)
char    display_getchar(int ptr);                               // Get the character at a specific screen position (CLI)
void    display_setcursor(uint8_t start, uint8_t end);          // Set cursor size (to disable, set both to 0) (CLI)
void    display_putcursor(int ptr);                             // Set the cursor to a specific screen position (CLI)
int     display_getcursor();                                    // Get the current position of the cursor (CLI)

void    display_putpixel(int x, int y, uint8_t c);              // Draw a pixel to specific position on the screen (GUI)
void    display_fillrect(int x, int y, int w, int h, int c);    // Draw a rectangle to specific position on the screen (GUI)

void    display_init();                                         // Initialize the display driver