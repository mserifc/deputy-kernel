#include "types.h"
#include "port.h"
#include "display.h"
#include "keyboard.h"
#include "common.h"

Port port = {
    port_inb,
    port_outb
};

Display display = {
    display_clear,
    display_reload,
    display_putchar,
    display_getchar,
    display_loadcursor,
    display_setTextColor,
    display_setBackgroundColor,
    display_getTextColor,
    display_getBackgroundColor
};

Keyboard keyboard = {
    keyboard_scankeycode,
    keyboard_scankey
};

void _kernel_main();
void _kernel_error(char* reason);

int command_handler(char* command) {
    if (strlen(command) <= 0) {
        putchar('\n');
        return 0;
    }
    char** argv = split(command);
    int argc = getStrTokenCount();
    if (strlen(argv[0]) <= 0) {
        putchar('\n');
        return 0;
    }
    if (argc > 0) {
        if (argv[0] && strcmp(argv[0], "echo") == 0) {
            for (int i = 1; i < argc; ++i) {
                printf(argv[i]);
                putchar(' ');
            }
            putchar('\n');
        } else if (argv[0] && strcmp(argv[0], "clear") == 0) {
            display.clear();
            setCursorLocation(0);
        } else if (argv[0] && strcmp(argv[0], "devtools") == 0) {
            if (argv[1] && strcmp(argv[1], "self_kill") == 0) {
                _kernel_error("INTENTIONAL_BAD_BEHAVIOR");
            } else {
                printf("Unknown developer tool.\n");
            }
        } else {
            printf("Command %s not found.\n", command);
        }
    } else {
        putchar('\n');
        return 0;
    }
    return 0;
}

void _kernel_error(char* reason) {
    display.setBackgroundColor(COLOR_BLUE);
    display.reload();
    display.setTextColor(COLOR_LIGHT_RED);
    printf("Critical error: %s", reason);
    while(1);
}

void _kernel_main() {
    port.outb(0x3D4, 0x0A);
    port.outb(0x3D5, 0x20);
    display.clear();
    puts("Serif's Kernel Build 21, booted successfully!\n");
    printf("My name is %s, thanks for trying!\n", "M. Serif C.");
    printf("I was born in %d, :D\n", 2009);
    printf("This kernel starts at %x, i guess... <:D\n", 0x14F0D1);
    printf("So I am %s and thats my kernel, i was born in %d, i guess my kernel start at %x, thats it! Thanks for trying! :)\n", "M. Serif C.", 2009, 0x14f0d1);
    printf("The kernel loaded %%100\n");
    puts("Welcome!\n");
    while (1) {
        char* prompt = scanf("# ");
        putchar('\n');
        if (command_handler(prompt) == -1) { /* Handle error */ }
    }
    while(1);
}