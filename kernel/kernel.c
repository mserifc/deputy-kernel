#include "types.h"
#include "common.h"

void _kernel_main() {
    port_outb(0x3D4, 0x0A);
    port_outb(0x3D5, 0x20);
    DisplayClear();
    printf("Serif's Kernel Build 19, booted successfully.\n");
    printf("Welcome!");
    while (1) {
        putchar('\n');
        char* prompt = scanf();
        putchar('\n');
        printf(prompt);
    }
    while(1);
}