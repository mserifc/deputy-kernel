#pragma once

#include "types.h"
#include "port.h"
#include "display.h"
#include "keyboard.h"

// Define the maximum length of the user input prompt
#define PROMPT_LENGTH 256

// Maximum tokens and digits constants for string manipulation
#define MAX_STR_TOKENS 127
#define MAX_INT_DIGITS 10

// Define macros for variable argument list handling
#define va_list __builtin_va_list                           // Type definition for variable argument list
#define va_start(ap, last) __builtin_va_start(ap, last)     // Initialize variable argument list
#define va_arg(ap, type) __builtin_va_arg(ap, type)         // Retrieve the next argument of a specified type
#define va_end(ap) __builtin_va_end(ap)                     // Clean up the variable argument list

// Function for update the cursor
int updateCursor();

// Function for set cursor position
void putcursor(int ptr);

// Functions for get string and digit counter values
int getStrTokenCount();     // Get string tokens
int getIntDigitCount();     // Get integer digits

// Function to introduce a delay (based on CPU speed)
void delay(uint32_t count);

// Function for scroll down the CLI Display
int scrolldown(int times);

// Function for check if a character is a digit (returns true if the character is a number)
bool isdigit(char chr);

// Functions for split strings and integers
char** split(char* str);    // String splitter
int* splitdigits(int num);  // Integer splitter

// Functions for memory manipulation
void* memset(void* ptr, char value, size_t size);       // Fill a block of memory with specific value
void* memcpy(void* dest, const void* src, size_t size); // Copy a block of memory from source to destination

// Functions for string manipulation
int strlen(char* str);                                      // Get the length of specific string
char* strcpy(char* dest, const char* src);                  // Copy a string to another one
int strcmp(const char* str1, const char* str2);             // Compare two strings
int snprintf(char* buffer, size_t size, char* format, ...); // Format and write output to a string

// Functions for string-integer manipulations
int atoi(const char* str);  // Convert ASCII character to integer
char* itoa(int num);        // Convert integer to ASCII character
char* xtoa(uint32_t num);   // Convert hexdecimal integer to ASCII character

// Functions for CLI (Command Line Interface) I/O
void putchar(char chr);         // Print a single character to the CLI output
void puts(char* str);           // Print a string to the CLI output
void printf(char* format, ...); // Print formatted output to the CLI output
char* scanf(char* header);      // Read user input and return the entered string
