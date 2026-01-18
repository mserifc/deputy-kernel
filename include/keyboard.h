#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "port.h"

typedef struct {
    uint8_t (*scankeycode)();
    char (*scankey)();
} Keyboard;

uint8_t keyboard_scankeycode();
char keyboard_scankey();

#endif // KEYBOARD_H