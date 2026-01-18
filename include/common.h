#include "types.h"
#include "display.h"
#include "keyboard.h"

#define PromptLength 256

void port_outb(uint16_t port, uint8_t value);
uint8_t port_inb(uint16_t port);

int strlen(char* str);
void printf(char* str);
void putchar(char chr);
char* scanf();