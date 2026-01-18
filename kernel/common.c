#include "common.h"

int CursorX = 0;
int CursorY = 0;

char ScannedPrompt[PromptLength];

int PromptPointer = 0;

void port_outb(uint16_t port, uint8_t value) {
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

uint8_t port_inb(uint16_t port) {
    uint8_t value;
    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

int strlen(char* str) {
    int len = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        len++;
    }
    return len;
}

void printf(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '\n') {
            CursorX = 0;
            CursorY++;
            continue;
        }
        if (CursorX >= DisplayWidth) {
            if (CursorY >= DisplayHeight) {
                for (int j = 0; j < DisplayHeight - 1; ++j) {
                    for (int k = 0; k < DisplayWidth; ++k) {
                        DisplayPutchar(DisplayGetchar(k, j+1), k, j);
                    }
                }
                for (int j = 0; j < DisplayWidth; j++) {
                    DisplayPutchar('\0', j, DisplayHeight-1);
                }
                CursorY = DisplayHeight - 1;
            } else {
                CursorY++;
            }
            CursorX = 0;
        }
        DisplayPutchar(str[i], CursorX, CursorY);
        CursorX++;
    }
}

void putchar(char chr) {
    if (chr == '\n') {
        CursorX = 0;
        CursorY++;
    } else if (CursorX >= DisplayWidth) {
        if (CursorY >= DisplayHeight) {
            for (int j = 0; j < DisplayHeight - 1; ++j) {
                for (int k = 0; k < DisplayWidth; ++k) {
                    DisplayPutchar(DisplayGetchar(k, j+1), k, j);
                }
            }
            for (int j = 0; j < DisplayWidth; j++) {
                DisplayPutchar('\0', j, DisplayHeight-1);
            }
            CursorY = DisplayHeight - 1;
        } else {
            CursorY++;
        }
        CursorX = 0;
    } else {
        DisplayPutchar(chr, CursorX, CursorY);
        CursorX++;
    }
}

char* scanf() {
    for (int i = 0; i < PromptLength; ++i) {
        ScannedPrompt[i] = '\0';
    }
    PromptPointer = 0;
    while(1) {
        char input = KeyboardScankey();
        if (input != '\0') {
            if (input == 0x0E && PromptPointer > 0) {
                PromptPointer--;
                ScannedPrompt[PromptPointer] = '\0';
                if (CursorX > 0) {
                    CursorX--;
                    DisplayPutchar('\0', CursorX, CursorY);
                } else if (CursorY >= 0) {
                    CursorY--;
                    CursorX = DisplayWidth - 1;
                }
            } else if (input == 0x1C) {
                ScannedPrompt[PromptLength - 1] = '\0';
                return ScannedPrompt;
            } else {
                ScannedPrompt[PromptPointer] = input;
                PromptPointer++;
                putchar(input);
            }
        }
    }
    return null;
}