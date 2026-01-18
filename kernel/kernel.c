#include "types.h"
#include "port.h"
#include "common.h"

void _kernel_error(char* str) {
    DisplaySettextcolor(COLOR_RED);
    printf("Critical Kernel Error: ");
    printf(str);
    while(1);
}

int commandhandler(char* command) {
    if (strlen(command) <= 0) {
        putchar('\0');
        return 0;
    }
    char** argv = split(command);
    int argc = gettokencount();
    return 0;
}

void _kernel_main() {
    port_outb(0x3D4, 0x0A);
    port_outb(0x3D5, 0x20);

    DisplayClear();
    printf("Serif's Kernel Build 20, booted successfully.\n");

    printf("Welcome!\n");
    while(1) {
        char* prompt = scanf("# ");
        putchar('\n');
        printf(prompt);
        putchar('\n');
        // if (commandhandler(prompt) != 0) { /* Handle error */ }
    }
    while(1);
}