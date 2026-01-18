#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"

#define DisplayWidth 80
#define DisplayHeight 25

enum DisplayColorTable {
    COLOR_BLACK = 0x00,
    COLOR_BLUE = 0x01,
    COLOR_GREEN = 0x02,
    COLOR_CYAN = 0x03,
    COLOR_RED = 0x04,
    COLOR_MAGENTA = 0x05,
    COLOR_BROWN = 0x06,
    COLOR_GRAY = 0x07,
    COLOR_DARK_GRAY = 0x08,
    COLOR_LIGHT_BLUE = 0x09,
    COLOR_LIGHT_GREEN = 0x0A,
    COLOR_LIGHT_CYAN = 0x0B,
    COLOR_LIGHT_RED = 0x0C,
    COLOR_LIGHT_MAGENTA = 0x0D,
    COLOR_YELLOW = 0x0E,
    COLOR_WHITE = 0x0F
};

typedef enum DisplayColorTable Color;

typedef struct {
    void (*clear)();
    void (*reload)();
    int (*putchar)(char chr, int point);
    char (*getchar)(int point);
    int (*loadcursor)(int point);
    void (*setTextColor)(Color color);
    void (*setBackgroundColor)(Color color);
    Color (*getTextColor)();
    Color (*getBackgroundColor)();
} Display;

void display_clear();
void display_reload();

int display_putchar(char chr, int point);
char display_getchar(int point);
int display_loadcursor(int point);

void display_setTextColor(Color color);
void display_setBackgroundColor(Color color);
Color display_getTextColor();
Color display_getBackgroundColor();

#endif // DISPLAY_H