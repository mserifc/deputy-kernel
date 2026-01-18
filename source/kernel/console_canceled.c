#include "kernel.h"

// Planning new console design

#define CONSOLE_SIGN "CONSOLE\0"

#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

typedef struct {
    char sign[8];
    int cursor;
    void* memory;
} console_t;

console_t* console_Active = NULL;

uint16_t* console_HWMemory = (uint16_t*)0xB8000;

int console_active(console_t* con) {
    if (con == NULL) { return; }
    if (ncompare(con->sign, CONSOLE_SIGN, sizeof(con->sign)) != 0) {
        ncopy(con->sign, CONSOLE_SIGN, sizeof(con->sign));
        fill(con->memory, 0, (CONSOLE_WIDTH * CONSOLE_HEIGHT) * sizeof(uint16_t));
    }
    if (con->memory == NULL) {
        con->memory = calloc(CONSOLE_WIDTH * CONSOLE_HEIGHT, sizeof(uint16_t));
        if (con->memory == NULL) { return -1; } con->cursor = 0;
    } else if (con->cursor < 0) { con->cursor = 0; }
    else if (con->cursor >= CONSOLE_WIDTH * CONSOLE_HEIGHT)
        { con->cursor = CONSOLE_WIDTH * CONSOLE_HEIGHT; }
    console_Active = con; return 0;
}

void console_clear(void) {
    if (!console_Active) { return; }
}

void console_print(const char* str, int len) {
    if (!console_Active) { return; }
}

void console_prompt(char* str, int len) {
    if (!console_Active) { return; }
}

void console_process(void) {
    if (!console_Active) { return; }
}