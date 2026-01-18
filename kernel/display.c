#include "display.h"

uint16_t* DisplayMemory = (uint16_t*)0xB8000;

Color DisplayTextColor = COLOR_WHITE;
Color DisplayBackgroundColor = COLOR_BLACK;

void display_clear() {
    for (int i = 0; i < (DisplayWidth * DisplayHeight); ++i) {
        DisplayMemory[i] =
            (((short)DisplayBackgroundColor & 0x000F) << 12) |
            (((short)DisplayTextColor & 0x000F) << 8) |
            (short)0x0000;
    };
}

void display_reload() {
    for (int i = 0; i < DisplayWidth * DisplayHeight; ++i) {
        DisplayMemory[i] =
            (((short)DisplayBackgroundColor & 0x0F) << 12) |
            (((short)DisplayTextColor & 0x0F) << 8) |
            (DisplayMemory[i] & 0x00FF);
    };
};

int display_putchar(char chr, int point) {
    if (point < 0 || point >= (DisplayWidth * DisplayHeight)) {
        return -1;
    }
    DisplayMemory[point] =
        (DisplayMemory[point] & 0xF000) |
        (((short)DisplayTextColor & 0x000F) << 8) |
        (short)chr;
    return 0;
}

char display_getchar(int point) {
    if (point < 0 || point >= (DisplayWidth * DisplayHeight)) {
        return -1;
    }
    return (char)(DisplayMemory[point] & 0x00FF);
}

int display_loadcursor(int point) {
    if (point < 0 || point >= (DisplayWidth * DisplayHeight)) {
        return -1;
    }
    for (int i = 0; i < (DisplayWidth * DisplayHeight); ++i) {
        DisplayMemory[i] =
            (((short)DisplayBackgroundColor & 0x0F) << 12) |
            (((short)DisplayTextColor & 0x0F) << 8) |
            (DisplayMemory[i] & 0x00FF);
    }
    DisplayMemory[point] =
        ((DisplayMemory[point] & 0x0F00) << 4) |
        ((DisplayMemory[point] & 0xF000) >> 4) |
        (DisplayMemory[point] & 0x00FF);
    return 0;
}

void display_setTextColor(Color color) { DisplayTextColor = color; };

void display_setBackgroundColor(Color color) { DisplayBackgroundColor = color; };

Color display_getTextColor() { return DisplayTextColor; };

Color display_getBackgroundColor() { return DisplayBackgroundColor; };