#include "keyboard.h"

bool KeyboardShiftstate = false;

const uint8_t KeyboardKeyscanTable[216] = {
     0 ,  0 , '1', '2', '3', '4', '5', '6', // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=',  0 ,  0 , // 0x08 - 0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', // 0x10 - 0x17
    'o', 'p', '[', ']', '\n', 0 , 'a', 's', // 0x18 - 0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', // 0x20 - 0x27
    '\'','`',  0 , '\\','z', 'x', 'c', 'v', // 0x28 - 0x2F
    'b', 'n', 'm', ',', '.', '/',  0 , '*', // 0x30 - 0x37
     0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 , // 0x38 - 0x3F
     0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7', // 0x40 - 0x47
    '8', '9', '-', '4', '5', '6', '+', '1', // 0x48 - 0x4F
    '2', '3', '0', '.',  0 ,  0 ,  0 ,  0 , // 0x50 - 0x57
};

const uint8_t KeyboardShiftscanTable[(sizeof(KeyboardKeyscanTable) / sizeof(KeyboardKeyscanTable[0]))] = {
     0 ,  0 , '!', '@', '#', '$', '%', '^', // 0x00 - 0x07
    '&', '*', '(', ')', '_', '+',  0 ,  0 , // 0x08 - 0x0F
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', // 0x10 - 0x17
    'O', 'P', '{', '}', '\n', 0 , 'A', 'S', // 0x18 - 0x1F
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', // 0x20 - 0x27
    '"', '~',  0 , '|', 'Z', 'X', 'C', 'V', // 0x28 - 0x2F
    'B', 'N', 'M', '<', '>', '?',  0 , '*', // 0x30 - 0x37
     0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 , // 0x38 - 0x3F
     0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7', // 0x40 - 0x47
    '8', '9', '-', '4', '5', '6', '+', '1', // 0x48 - 0x4F
    '2', '3', '0', '.',  0 ,  0 ,  0 ,  0 , // 0x50 - 0x57
};

uint8_t keyboard_scankeycode() {
    uint8_t current_input, new_input;
    current_input = port_inb(0x60);
    while(1) {
        new_input = port_inb(0x60);
        if (current_input != new_input) {
            return new_input;
        }
    }
}

char keyboard_scankey() {
    uint8_t input = keyboard_scankeycode();
    if (input == 0x2A || input == 0x36) {
        KeyboardShiftstate = true;
        return '\0';
    }
    if (input == 0xAA || input == 0xB6) {
        KeyboardShiftstate = false;
        return '\0';
    }
    if (
        input == 0x0E ||
        input == 0x1C
    ) {
        return (char)input;
    }
    if (input < (sizeof(KeyboardKeyscanTable) / sizeof(KeyboardKeyscanTable[0]))) {
        if (!KeyboardShiftstate) {
            return (char)KeyboardKeyscanTable[input];
        } else if (KeyboardShiftstate) {
            return (char)KeyboardShiftscanTable[input];
        }
    }
    return -1;
}