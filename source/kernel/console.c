#include "kernel.h"

#include "hw/port.h"

#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

bool console_Active = true;

bool console_HardSerial = false;

uint16_t* console_Memory = (uint16_t*)0xB8000;

uint8_t console_Color = 0x07;

uint16_t console_Cursor = (uint16_t)-1;

void console_update(void) {
    if (!console_Active) { return; }
    if (console_Cursor == (uint16_t)-1) {
        size_t cur = 0;
        port_outb(0x3D4, 0x0F);
        cur |= port_inb(0x3D5);
        port_outb(0x3D4, 0x0E);
        cur |= ((size_t)port_inb(0x3D5)) << 8;
        console_Cursor = cur;
    } else {
        if (console_Cursor >= CONSOLE_WIDTH * CONSOLE_HEIGHT) {
            console_Cursor = (CONSOLE_WIDTH * CONSOLE_HEIGHT) - 1;
        }
        port_outb(0x3D4, 0x0F);
        port_outb(0x3D5, (uint8_t)(console_Cursor & 0xFF));
        port_outb(0x3D4, 0x0E);
        port_outb(0x3D5, (uint8_t)((console_Cursor >> 8) & 0xFF));
    }
}

void console_scroll(int times) {
    if (!console_Active) { return; }
    if (times < 0) { return; }
    for (int i = 0; i < times; ++i) {
        for (int j = 0; j < (CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1)); ++j) {
            console_Memory[j] = (console_Memory[j + CONSOLE_WIDTH]);
        }
        for (int j = ((CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH); j < CONSOLE_WIDTH * CONSOLE_HEIGHT; ++j) {
            console_Memory[j] = (uint16_t)(console_Color << 8) | '\0';
        }
    }
}

void console_clear(void) {
    if (!console_Active) { return; }
    console_update();
    for (size_t i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; ++i) { console_Memory[i] = 0; }
    console_Cursor = 0;
    console_update();
}

void console_print(const char* str, int len) {
    if (!console_Active) { return; }
    console_update();
    for (int i = 0; i < len; ++i) {
        if (str[i] == '\0') { return; }
        if (console_HardSerial) {
            if (str[i] == '\n') {
                port_outb(0x3F8, (uint8_t)'\r');
                port_outb(0x3F8, (uint8_t)'\n');
            } else {
                port_outb(0x3F8, (uint8_t)str[i]);
            }
        }
        if (str[i] == '\n') {
            console_Cursor = (console_Cursor / CONSOLE_WIDTH + 1) * CONSOLE_WIDTH;
        } else if (str[i] == '\t') {
            int tablen = 8 - (console_Cursor % 8);
            for (int j = 0; j < tablen; ++j) {
                console_Memory[console_Cursor] = (uint16_t)(console_Color << 8) | ' ';
                console_Cursor++;
            }
        } else {
            console_Memory[console_Cursor] = (uint16_t)(console_Color << 8) | str[i];
            console_Cursor++;
        }
        if (console_Cursor >= CONSOLE_WIDTH * CONSOLE_HEIGHT) {
            console_scroll(1);
            console_Cursor = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
        }
    }
    console_update();
}

// void console_prompt(char* str, int len) { return; }