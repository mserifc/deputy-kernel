#include "display.h"

uint16_t* DisplayMemory = (uint16_t*)0xB8000;

Color DisplayTextColor = COLOR_WHITE;
Color DisplayBackgroundColor = COLOR_BLACK;

void DisplayClear() {
    for (int i = 0; i < (DisplayWidth * DisplayHeight); ++i) {
        DisplayMemory[i] = (((short)DisplayBackgroundColor & 0x000F) << 12) | (((short)DisplayTextColor & 0x000F) << 8) | (short)0x0000;
    }
}

int DisplayPutchar(char chr, int x, int y) {
    if (x < 0 && y < 0 && x >= DisplayWidth && y >= DisplayHeight) {
        return 1;
    }
    int point = (y * DisplayWidth) + x;
    if (point < 0 && point >= (DisplayWidth * DisplayHeight)) {
        return 1;
    }
    DisplayMemory[point] = (DisplayMemory[point] & 0xF000) | (((short)DisplayTextColor & 0x000F) << 8) | (short)chr;
    return 0;
}

char DisplayGetchar(int x, int y) {
    if (x < 0 && y < 0 && x >= DisplayWidth && y >= DisplayHeight) {
        return 1;
    }
    int point = (y * DisplayWidth) + x;
    if (point < 0 && point >= (DisplayWidth * DisplayHeight)) {
        return 1;
    }
    return (char)(DisplayMemory[point] & 0x00FF);
}

int DisplayPutcursor(int x, int y) {
    if (x < 0 && y < 0 && x >= DisplayWidth && y >= DisplayHeight) {
        return 1;
    }
    int point = (y * DisplayWidth) + x;
    if (point < 0 && point >= (DisplayWidth * DisplayHeight)) {
        return 1;
    }
    for (int i = 0; i < (DisplayWidth * DisplayHeight); ++i) {
        DisplayMemory[i] = (((short)DisplayBackgroundColor & 0x0F) << 12) | (((short)DisplayTextColor & 0x0F) << 8) | (DisplayMemory[i] & 0x00FF);
    }
    DisplayMemory[point] = ((DisplayMemory[point] & 0x0F00) << 4) | ((DisplayMemory[point] & 0xF000) >> 4) | (DisplayMemory[point] & 0x00FF);
    return 0;
}