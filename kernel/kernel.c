#include "kernel.h"
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

Memory memory = {
    memory_init,
    memory_allocate,
    memory_free,
    memory_report
};

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
            if (argv[1] && strcmp(argv[1], "self_destroy") == 0) {
                _kernel_panic("Manually triggered");
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

void _kernel_main() {
    puts("Welcome!\n");
    char* my_data = (char*)memory.allocate();
    // strcpy(my_data, "Hello, World! What's up?");
    snprintf(my_data, 4096, "Hello, %s!", "World");
    printf("My data: %s\n", my_data);
    printf("Memory usage: %s\n", memory.report());
    memory.free(my_data);
    printf("Allocated memory freed.\n");
    printf("Memory usage: %s\n", memory.report());
    puts("Unable to run user shell, switching to built-in kernel shell.\n");
    while (1) {
        char* prompt = scanf("# ");
        putchar('\n');
        if (prompt && strcmp(prompt, "exit") == 0) { break; }
        if (command_handler(prompt) == -1) { /* Handle error */ }
    }
    _kernel_panic("No processes to execute");
}

void _kernel_init() {
    port.outb(0x3D4, 0x0A);
    port.outb(0x3D5, 0x20);
    display.clear();
    memory_init();
    puts("Serif's Kernel Build 22, booted successfully!\n");
    _kernel_main();
}