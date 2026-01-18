#ifndef COMMON_H
#define COMMON_H

#include "types.h"
#include "port.h"
#include "display.h"
#include "keyboard.h"

#define PromptLength 256

#define MaxStrTokens 127
#define MaxIntDigits 10

#define va_list __builtin_va_list
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

int setCursorLocation(int point);
int setCursorVisibility(bool toggle);
int getCursorLocation();

int getStrTokenCount();
int getIntDigitCount();

int scrolldown(int times);

bool isdigit(char chr);
char** split(char* str);
int* splitdigits(int num);

char* strreset(char* str);
int strlen(char* str);
char* strcpy(char* dest, const char* src);
int strcmp(const char* str1, const char* str2);

int atoi(const char* str);
char* itoa(int num);
char* itos(int num);
char* xtoa(uint32_t num);

void putchar(char chr);
void puts(char* str);
void printf(char* format, ...);
char* scanf(char* header);

#endif // COMMON_H