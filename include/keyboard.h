#pragma once

#include "types.h"
#include "port.h"
#include "interrupts.h"
#include "common.h"

#define KEYBOARD_PORT 0x60

uint8_t keyboard_scankeycode();
char keyboard_scankey();

// void keyboard_init();
