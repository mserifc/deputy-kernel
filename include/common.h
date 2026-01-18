#include "types.h"
#include "port.h"
#include "display.h"
#include "keyboard.h"

#define PromptLength 256

#define MaxStrTokens 127
#define MaxIntDigits 10

void delay(uint32_t ms);

int getcursorlocation();
int setcursorlocation(int point);
int setcursorvisibility(bool toggle);
int gettokencount();

void scrolldown(int times);

bool isdigit(char chr);
int atoi(const char* str);

int strlen(char* str);
char* strcpy(char* dest, const char* src);
int strcmp(const char* str1, const char* str2);
char** split(char* str);

void printf(char* str);
void putchar(char chr);
char* scanf(char* promptheader);