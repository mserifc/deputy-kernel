#pragma once

#include "types.h"
#include "hardware/port.h"
#include "drivers/display.h"
#include "drivers/keyboard.h"

// * Constants

#define UTILS_RTC_INDEXPORT         0x70    // RTC (Real-Time Clock) index port
#define UTILS_RTC_DATAPORT          0x71    // RTC (Real-Time Clock) data port

#define UTILS_SPLIT_MAXTOKENCOUNT   256     // Limit of token count for split function
#define UTILS_SPLIT_MAXTOKENLENGTH  128     // Limit of token length for split function

#define UTILS_TABLENGTH             8       // Tabulation length

#define UTILS_PROMPTBUFFER_LENGTH   256     // Maximum length of prompt buffer

// * Types

// Date type for represent date values
typedef struct {
    uint8_t sec;        // Second
    uint8_t min;        // Minute
    uint8_t hour;       // Hour
    uint8_t day;        // Day
    uint8_t mon;        // Month
    uint16_t year;      // Year
} date_t;

// Tokens type for represent splitted string tokens
typedef struct {
    int c;                              // Token count
    char v                              // Token vector
        [UTILS_SPLIT_MAXTOKENCOUNT]     // Index of words
        [UTILS_SPLIT_MAXTOKENLENGTH];   // Index of characters
} tokens_t;

// * Public functions

// Timing functions

date_t      date();                     // Get RTC date
void        delay(uint32_t count);      // Introduce a delay (based on CPU speed)
void        sleep(uint32_t sec);        // Sleep the system for a certain amount of time (based on RTC)

// Memory manipulation functions

void*       fill(void* ptr, char chr, size_t len);      // Fill a block of memory with specific value
char*       copy(char* dest, char* src);                // Copy a block of memory from source to destination
void*       ncopy(void* dest, void* src, size_t len);   // Copy a block of memory from source to destination with limit

// Comparison functions

int         compare(char* str1, char* str2);                // Compare two strings
int         ncompare(void* ptr1, void* ptr2, size_t len);   // Compare two buffers with limit

// String manipulation functions

int         length(char* str);              // Get length of specific string
tokens_t    split(char* str, char deli);    // Splits a string by a deliminer

// String-integer conversion functions

int         convert_atoi(char* str);        // Convert ASCII string to integer
char*       convert_itoa(int num);          // Convert integer to ASCII string
char*       convert_xtoa(uint32_t num);     // Convert hexadecimal integer to ASCII string

// I/O functions

int         getcursor();                                                // Get position of cursor position
void        putcursor(int ptr);                                         // Set position of cursor position
void        putchar(char chr);                                          // Print a single character to the CLI output
void        puts(char* str);                                            // Print a string to the CLI output
void        printf(char* format, ...);                                  // Print formatted output to the CLI output
int         snprintf(char* buffer, size_t size, char* format, ...);     // Format and write output to a string
char*       prompt(char* header);                                       // Read user input and return the entered string