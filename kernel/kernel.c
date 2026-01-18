#include "kernel.h"

#include "multiboot.h"

#include "hardware/port.h"
#include "hardware/protect.h"
#include "hardware/interrupts.h"

#include "drivers/display.h"
#include "drivers/keyboard.h"

#include "filesystem/ramfs.h"

// Kernel physical size
size_t kernel_PhysicalSize;

// Memory Lower (Below 1MB) and Upper (Above 1MB) Sizes
size_t kernel_MemoryLowerSize;  // Memory lower size
size_t kernel_MemoryUpperSize;  // Memory upper size

// Kernel boot device
uint32_t kernel_BootDevice;

// Boot devices list
char* kernel_BootDeviceList[] = {
    "Invalid",
    "Harddisk",
    "Floppy",
    "CDROM",
    "USB",
    "Network",
    "SCSI"
};

// Home path for built-in kernel shell
char* kernel_HomePath = "/";

// Working path buffer for built-in kernel shell
char kernel_WorkingPath[RAMFS_MAX_PATH_LENGTH];

// Function for get kernel size
size_t kernel_size() { return kernel_PhysicalSize; }

// Function for get system memory size
size_t kernel_memsize() { return kernel_MemoryLowerSize + kernel_MemoryUpperSize; }

// Function for get kernel boot device
uint32_t kernel_bootdev() { return kernel_BootDevice; }

void test(void) {
    printf("Merhaba, dunya!\n"); exit();
}

void test1() {
    while (true) {
        printf("Example task 1 running...\n");
        sleep(1);
        yield();
        printf("Returned to example task 1\n");
    }
}

void test2() {
    while (true) {
        printf("Example task 2 working...\n");
        sleep(1);
        yield();
        printf("Returned to example task 2\n");
    }
}

void test3() {
    while (true) {
        printf("Example task 3 in focus...\n");
        sleep(1);
        yield();
        printf("Returned to example task 3\n");
    }
}

void testshell() {
    int n = 0;
    display_clear(); putcursor(0); puts("# ");
    while (true) { sleep(1); if (n == 10) { puts(ramfs_readFile("/dev/keyboard")); } ++n; yield(); }
}

/**
 * @brief Built-in kernel text editor
 * 
 * @param path Path of file for edit
 * 
 * @return Exit code
 */
int kernel_textEditor(char* path) {
    int new = false;
    int ptrsave = getcursor(); char* scsave = (char*)malloc(DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT);
    if (scsave == NULL) { printf("Out of memory\n"); return EXIT_FAILURE; }
    fill(scsave, 0, DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT);
    for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) { scsave[i] = display_getchar(i); }
    int ptr = 0; int size = 0; char* buffer = (char*)malloc(KERNEL_TEXTEDIT_BUFFSIZE);
    if (buffer == NULL) { printf("Out of memory\n"); free(scsave); return EXIT_FAILURE; }
    if (path != NULL) {
        ramfs_Entry_t* file = ramfs_stat(path);
        if (file == NULL) { printf("No such file or directory: %s\n", path); return EXIT_FAILURE; }
        if (file->type == RAMFS_TYPE_DIR) { printf("It's a directory\n"); return EXIT_FAILURE; }
        // if ((((file->perm >> 6) & 7) & 2) == 0) { printf("Permission denied\n"); return EXIT_PERMISSION_DENIED; }
        if (file->size > KERNEL_TEXTEDIT_BUFFSIZE) { printf("File too large to edit\n"); return EXIT_FAILURE; }
        fill(buffer, 0, KERNEL_TEXTEDIT_BUFFSIZE); ncopy(buffer, ramfs_readFile(path), file->size);
        size = file->size;
    } else { new = true; }
    bool edit = false; while (true) { display_clear(); putcursor(0);
        int scstart = 0; int scend = size;
        int ptrline = 0, linesize = 0;
        for (int i = 0; i < ptr; ++i) {
            if (buffer[i] == '\n' || linesize >= DISPLAY_CLIWIDTH) {
                ++ptrline; linesize = 0;
            } else { ++linesize; }
        }
        int totallines = 1; linesize = 0;
        for (int i = 0; i < size; ++i) {
            if (buffer[i] == '\n' || linesize >= DISPLAY_CLIWIDTH) {
                ++totallines; linesize = 0;
            } else { ++linesize; }
        }
        int startline = ptrline - DISPLAY_CLIHEIGHT / 2;
        if (startline < 0) { startline = 0; }
        if (startline > totallines - DISPLAY_CLIHEIGHT)
            { startline = totallines - DISPLAY_CLIHEIGHT; }
        if (startline < 0) { startline = 0; }
        int currentline = 0; linesize = 0; scstart = 0;
        for (int i = 0; i < size; ++i) {
            if (currentline == startline) { scstart = i; break; }
            if (buffer[i] == '\n' || linesize >= DISPLAY_CLIWIDTH) {
                ++currentline; linesize = 0;
            } else { ++linesize; }
        }
        currentline = 0; scend = size; linesize = 0;
        for (int i = scstart; i < size; ++i) {
            if (currentline == DISPLAY_CLIHEIGHT) { scend = i; break; }
            if (buffer[i] == '\n' || linesize >= DISPLAY_CLIWIDTH) {
                ++currentline; linesize = 0;
            } else { ++linesize; }
        }
        for (int i = scstart; i < scend; ++i) {
            if (i >= scend - 1 && buffer[i] == '\n') { break; }
            if (buffer[i] != '\0') { putchar(buffer[i]); }
        } { int scptr = 0;
            for (int i = scstart; i < ptr; ++i) {
                if (buffer[i] == '\n') {
                    scptr = (scptr / DISPLAY_CLIWIDTH + 1) * DISPLAY_CLIWIDTH;
                } else if (buffer[i] != '\0') { ++scptr; }
            } putcursor(scptr); }
        key_t key = keyboard_waitkey();
        if (key == KEYBOARD_KEY_ESCAPE) {
            if (edit) {
                display_clear(); putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));
                printf("Do you want to save changes? (Y/N/C) ");
                escape_operation:
                key = keyboard_waitkey();
                if (key == KEYBOARD_KEY_Y) {
                    char* writepath = path;
                    if (new) {
                        display_clear(); putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));
                        char* input = prompt("New file name: ");
                        if (split(input, ' ').c == 1) {
                            char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH);
                            if (buffer == NULL) { printf(" Error: Out of memory"); sleep(2); continue; }
                            if (input[0] == '/') { copy(input, buffer); } else {
                                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, input);
                            }
                            if (ramfs_stat(buffer) != NULL) {
                                if (ramfs_stat(buffer)->type != RAMFS_TYPE_FILE)
                                    { printf(" Error: It's a directory"); sleep(2); continue; }
                                display_clear(); putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));
                                printf("Already exists, would you like to overwrite it? (Y/N/C) ");
                                int key2 = keyboard_waitkey();
                                if (key2 == KEYBOARD_KEY_Y) {
                                    //
                                } else if (key2 == KEYBOARD_KEY_N) {
                                    break;
                                } else { continue; }
                            }
                            writepath = buffer;
                        } else { printf(" Error: Unacceptable name"); sleep(2); continue; }
                    }
                    int n = ramfs_writeFile(writepath, size, buffer);
                    if (n != RAMFS_STATUS_SUCCESS) {
                        if (n == RAMFS_STATUS_OUTOFMEMORY) { printf(" Error: Out of memory"); sleep(2); continue; }
                        if (n == RAMFS_STATUS_NOTFILE) { printf(" Error: It's a directory"); sleep(2); continue; }
                        if (n == RAMFS_STATUS_PATHNOTFOUND) { printf(" Error: Path not found"); sleep(2); continue; }
                        printf(" Error occurred while writing file"); sleep(2); continue;
                    } break;
                } else if (key == KEYBOARD_KEY_N) {
                    break;
                } else if (key == KEYBOARD_KEY_C) {
                    continue;
                } goto escape_operation;
            } else { break; }
        } else if (key == KEYBOARD_KEY_SP_CURSORUP) {
            int ptrlinelen = 0, upperlinelen = 0;
            for (int i = 0; ptr - i > 0 && (i == 0 || buffer[ptr - i] != '\n'); ++i) { ++ptrlinelen; } 
            ptr -= ptrlinelen;
            if (buffer[ptr - 1] == '\n' || buffer[ptr - 1] == '\0') { continue; }
            for (int i = 0; i == 0 || (buffer[ptr - i] != '\0' && buffer[ptr - i] != '\n'); ++i) { ++upperlinelen; }
            if (upperlinelen < ptrlinelen) { continue; } ptr -= upperlinelen - ptrlinelen;
        } else if (key == KEYBOARD_KEY_SP_CURSORLEFT) {
            if (ptr > 0) { --ptr; }
        } else if (key == KEYBOARD_KEY_SP_CURSORDOWN) {
            int ptrlinelen = 0, lowerlinelen = 0;
            for (int i = 0; ptr - i >= 0 && (i == 0 || buffer[ptr - i] != '\n'); ++i) { ++ptrlinelen; }
            while (ptr < size && buffer[ptr] != '\n') { ++ptr; }
            for (int i = 0; ptr + i < size && (i == 0 || (buffer[ptr + i] != '\0' && buffer[ptr + i] != '\n')); ++i)
                { ++lowerlinelen; }
            if (lowerlinelen < ptrlinelen) { ptr += lowerlinelen; continue; }
            ptr += ptrlinelen;
        } else if (key == KEYBOARD_KEY_SP_CURSORRIGHT) {
            if (ptr < size) { ++ptr; }
        } else if (key == KEYBOARD_KEY_BACKSPACE) {
            if (!edit) { edit = true; }
            if (ptr > 0) {
                for (int i = ptr - 1; i < size - 1; ++i) {
                    buffer[i] = buffer[i + 1];
                } --ptr; --size;
            }
        } else if (key == KEYBOARD_KEY_TABULATION) {
            if (!edit) { edit = true; }
            int tablen = UTILS_TABLENGTH - (getcursor() % UTILS_TABLENGTH);
            if (size + tablen <= KERNEL_TEXTEDIT_BUFFSIZE) {
                for (int i = 0; i < tablen; ++i) {
                    for (int j = size; j > ptr; --j) {
                        buffer[j] = buffer[j - 1];
                    } buffer[ptr] = ' '; ++ptr; ++size;
                }
            }
        } else if (key == KEYBOARD_KEY_ENTER) {
            if (!edit) { edit = true; }
            if (size < KERNEL_TEXTEDIT_BUFFSIZE) {
                for (int i = size; i > ptr; --i) {
                    buffer[i] = buffer[i - 1];
                } buffer[ptr] = '\n'; ++ptr; ++size;
            }
        } else if (keyboard_tochar(key) != 0) {
            if (!edit) { edit = true; }
            if (size < KERNEL_TEXTEDIT_BUFFSIZE) {
                for (int i = size; i > ptr; --i) {
                    buffer[i] = buffer[i - 1];
                } buffer[ptr] = keyboard_tochar(key); ++ptr; ++size;
            }
        }
    } for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) {
        display_putchar(scsave[i], i);
    } putcursor(ptrsave);
    free(buffer); free(scsave); return EXIT_SUCCESS;
}

/**
 * @brief Built-in kernel command handler
 * 
 * @param path Working directory
 * @param cmd Command to execute
 * 
 * @return Exit code
 */
int kernel_commandHandler(char* path, char* cmd) {
    if (length(cmd) <= 0) { return 0; }
    tokens_t arg = split(cmd, ' ');
    if (arg.c > 0) {
        /* --> */ if (arg.v[0] && compare(arg.v[0], "echo") == 0) {
            for (int i = 1; i < arg.c; ++i) {
                printf(arg.v[i]);
                if (i+1 != arg.c){ putchar(' '); }
            } putchar('\n'); return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "clear") == 0) {
            display_clear(); putcursor(0); return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "pwd") == 0) {
            printf("%s\n", path); return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "cd") == 0) {
            if (arg.c < 2) {
                fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH);
                copy(kernel_WorkingPath, kernel_HomePath);
                return EXIT_SUCCESS;
            } if (arg.v[1] && compare(arg.v[1], ".") == 0) {
                return EXIT_SUCCESS;
            } else if (arg.v[1] && compare(arg.v[1], "..") == 0) {
                tokens_t path = split(kernel_WorkingPath, '/');
                int ptr = 0; for (int i = 0; i < path.c - 1; ++i) { ptr += length(path.v[i]) + 1; }
                fill(&kernel_WorkingPath[ptr+1], 0, length(&kernel_WorkingPath[ptr]));
                return EXIT_SUCCESS;
            }
            if (length(arg.v[1]) >= RAMFS_MAX_PATH_LENGTH) { printf("Path too long: %s\n", arg.v[1]); return EXIT_FAILURE; }
            if (arg.v[1][0] != '/') {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg.v[1][length(arg.v[1]) - 1] == '/') {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg.v[1]);
                } else { snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg.v[1]); }
                if (ramfs_stat(buffer) == NULL)
                    { printf("No such directory: %s\n", arg.v[1]); return EXIT_FAILURE; }
                fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, buffer);
                free(buffer); return EXIT_SUCCESS;
            }
            if (arg.v[1][length(arg.v[1]) - 1] != '/') {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s/", arg.v[1]);
                if (ramfs_stat(buffer) == NULL)
                    { printf("No such directory: %s\n", arg.v[1]); return EXIT_FAILURE; }
                fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, buffer);
                free(buffer); return EXIT_SUCCESS;
            }
            if (ramfs_stat(arg.v[1]) == NULL) { printf("No such directory: %s\n", arg.v[1]); return EXIT_FAILURE; }
            fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, arg.v[1]); return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "ls") == 0) {
            char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
            if (arg.c < 2) { copy(buffer, path); } else {
                if (arg.v[1][0] == '/') { copy(arg.v[1], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg.v[1]);
                }
            }
            if (ramfs_stat(buffer) == NULL) { printf("No such directory: %s\n", buffer); free(buffer); return EXIT_FAILURE; }
            int* ents = ramfs_readDir(buffer);
            if (ents == NULL) { printf("Unable to read directory: %s\n", buffer); return EXIT_FAILURE; }
            for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT && ents[i] != RAMFS_DIRENTEND; ++i) {
                ramfs_Entry_t* ent = ramfs_dirent(ents[i]);
                if (
                    ent->name && compare(ent->name, buffer) == 0 ||
                    split(ent->name, '/').c > split(buffer, '/').c + 1
                ) { continue; }
                if (ent) { printf("%s\n", &ent->name[length(buffer)]); }
            } free(buffer); return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "cat") == 0) {
            if (arg.c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            if (arg.v[1][0] == '/') {
                if (ramfs_stat(arg.v[1]) == NULL || ramfs_stat(arg.v[1])->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg.v[1]); return EXIT_FAILURE; }
                char* data = ramfs_readFile(arg.v[1]);
                for (int i = 0; i < ramfs_stat(arg.v[1])->size; ++i) { if (data[i] != '\0') { putchar(data[i]); } }
                putchar('\n'); return EXIT_SUCCESS;
            } else {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg.v[1]);
                if (ramfs_stat(buffer) == NULL || ramfs_stat(buffer)->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg.v[1]); return EXIT_FAILURE; }
                char* data = ramfs_readFile(buffer);
                for (int i = 0; i < ramfs_stat(buffer)->size; ++i) { if (data[i] != '\0') { putchar(data[i]); } }
                putchar('\n'); free(buffer); return EXIT_SUCCESS;
            }
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "mkdir") == 0) {
            if (arg.c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            for (int i = 1; i < arg.c; ++i) {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg.v[i][0] == '/') { copy(arg.v[i], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg.v[i]);
                }
                if (ramfs_stat(buffer) == NULL) {
                    if (ramfs_createDir(buffer) != RAMFS_STATUS_SUCCESS) { printf("Unable to make: %s\n", buffer); }
                } else { printf("Already exists: %s\n", buffer); } free(buffer);
            } return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "touch") == 0) {
            if (arg.c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            for (int i = 1; i < arg.c; ++i) {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg.v[i][0] == '/') { copy(arg.v[i], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg.v[i]);
                }
                if (ramfs_stat(buffer) != NULL) {
                    ramfs_stat(buffer)->mtime = date();
                    ramfs_stat(buffer)->atime = date();
                } else {
                    if (ramfs_writeFile(buffer, 1, " ") != RAMFS_STATUS_SUCCESS) {
                        printf("Unable to write %s\n", buffer);
                    } free(buffer);
                }
            } return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "rmdir") == 0) {
            if (arg.c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            for (int i = 1; i < arg.c; ++i) {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg.v[i][0] == '/') { copy(arg.v[i], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg.v[i]);
                }
                if (ramfs_stat(buffer) != NULL) {
                    if (ramfs_stat(buffer)->type != RAMFS_TYPE_DIR) { printf("Not a directory: %s\n", buffer); }
                    bool empty = true;
                    int* ents = ramfs_readDir(buffer);
                    if (ents == NULL) { printf("Unable to read directory: %s\n", buffer); return EXIT_FAILURE; }
                    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT && ents[i] != RAMFS_DIRENTEND; ++i) {
                        //
                    }
                    if (ramfs_remove(buffer) != RAMFS_STATUS_SUCCESS) { printf("Unable to remove %s\n", buffer); }
                } else { printf("No such directory: %s\n", buffer); } free(buffer);
            } return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "rm") == 0) {
            if (arg.c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            for (int i = 1; i < arg.c; ++i) {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg.v[i][0] == '/') { copy(arg.v[i], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg.v[i]);
                }
                if (ramfs_stat(buffer) != NULL) {
                    if (ramfs_stat(buffer)->type == RAMFS_TYPE_DIR) { printf("It's a directory: %s\n", buffer); }
                    if (ramfs_remove(buffer) != RAMFS_STATUS_SUCCESS) { printf("Unable to remove %s\n", buffer); }
                } else { printf("No such file: %s\n", buffer); } free(buffer);
            } return EXIT_SUCCESS;
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "edit") == 0) {
            if (arg.c < 2) { return kernel_textEditor(NULL); }
            if (arg.v[1][0] == '/') {
                if (ramfs_stat(arg.v[1]) == NULL || ramfs_stat(arg.v[1])->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg.v[1]); return EXIT_FAILURE; }
                return kernel_textEditor(arg.v[1]);
            } else {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg.v[1]);
                if (ramfs_stat(buffer) == NULL || ramfs_stat(buffer)->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg.v[1]); return EXIT_FAILURE; }
                return kernel_textEditor(buffer);
            }
        } /* --> */ else if (arg.v[0] && compare(arg.v[0], "uname") == 0) {
            printf("Deputy\n"); return EXIT_SUCCESS;
        } /* --> */ else {
            printf("Command %s not found\n", arg.v[0]); return EXIT_COMMAND_NOT_FOUND;
        }
    } else { return 0; }
}

// Main function of kernel
void kernel_main(void) {
    puts("Welcome!\n");     // Print welcome message for user
    putchar('\n');

    // * Memory manager test
    if (false) {
        puts("---- Memory test started ----\n");
        printf("Memory usage: %s\n", memory_report());
        char* my_data = (char*)malloc(9*1024);
        if (my_data == NULL) { kernel_panic("Memory test error: Allocated memory is null"); }
        printf("Allocated a memory block.\n");
        // copy(my_data, "Hello, World!");
        snprintf(my_data, MEMORY_BLOCKSIZE, "Hello, %s!", "World");
        printf("Writed data to allocated memory block.\n");
        printf("My data: %s\n", my_data);
        printf("Memory usage: %s\n", memory_report());
        free(my_data);
        printf("Allocated memory freed.\n");
        printf("Memory usage: %s\n", memory_report());
        puts("---- Memory test ended ----\n");
        putchar('\n');
    }

    // * File system test
    if (false) {
        puts("---- File system test started ----\n");
        int status;
        status = ramfs_createDir("/home/");
        printf("Create /home/: %d\n", status); // !
        status = ramfs_createDir("/home/user/");
        printf("Create /home/user/: %d\n", status); // !
        char* content = "Hello, world!";
        status = ramfs_writeFile("/home/user/test.txt", 15, content);
        printf("Write /home/user/test.txt: %d\n", status); // !
        char* data = ramfs_readFile("/home/user/test.txt");
        if (data) {
            printf("Read /home/user/test.txt: %s\n", data);
        } else {
            printf("Read /home/user/test.txt: FAILED\n");
        } // !
        int* entries = ramfs_readDir("/home/user/");
        if (entries) {
            printf("Entries under /home/user/:\n");
            for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT && entries[i] != 0; ++i) {
                ramfs_Entry_t* ent = ramfs_dirent(entries[i]);
                if (ent) {
                    printf(" - %s (%d)\n", ent->name, ent->type);
                }
            }
        } else {
            printf("ReadDir /home/user/: FAILED\n");
        } // !
        status = ramfs_remove("/home/user/test.txt");
        printf("Remove /home/user/test.txt: %d\n", status); // !
        data = ramfs_readFile("/home/user/test.txt");
        if (data) {
            printf("Unexpected Read: %s\n", data);
        } else {
            printf("Read deleted file: Correctly failed\n");
        } // !
        status = ramfs_remove("/home/");
        printf("Try remove /home/: %d\n", status); // !
        status = ramfs_remove("/home/user/");
        printf("Remove /home/user/: %d\n", status); // !
        status = ramfs_remove("/home/");
        printf("Remove /home/: %d\n", status); // !
        status = ramfs_remove("/");
        printf("Remove root (/): %d\n", status); // !
        puts("---- File system test ended ----\n");
        putchar('\n');
    }

    // * Built-in kernel shell
    if (true) {
        fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, "/");
        if (ramfs_createDir("/system/") != RAMFS_STATUS_SUCCESS) { printf("Unable to create /system/\n"); }
        if (ramfs_writeFile("/system/hello.txt", length("Hello, world!"), "Hello, world!") != RAMFS_STATUS_SUCCESS)
            { printf("Unable to write /system/hello.txt\n"); }
        if (ramfs_createDir("/myfiles/") != RAMFS_STATUS_SUCCESS) { printf("Unable to create /myfiles/\n"); }
        if (ramfs_createDir("/myfiles/docs/") != RAMFS_STATUS_SUCCESS) { printf("Unable to create /myfiles/docs/\n"); }
        if (ramfs_writeFile("/welcome.txt", length("Welcome!"), "Welcome!") != RAMFS_STATUS_SUCCESS)
            { printf("Unable to write /init\n"); }
        puts("Unable to run user shell, switching to built-in kernel shell.\n");
        while (true) {
            printf("%s ", kernel_WorkingPath); char* cmd = prompt("# "); putchar('\n');
            if (split(cmd, ' ').v[0] && compare(split(cmd, ' ').v[0], "exit") == 0)
                { printf("Process completed\n"); break; }
            if (kernel_commandHandler(kernel_WorkingPath, cmd) != EXIT_SUCCESS) { /* Handle error */ }
        }
    }
    // exit();

    kernel_panic("No processes to execute");    // Switch to idle if no process
}

// Initialize function of kernel
void kernel_init(multiboot_info_t* boot_info, multiboot_uint32_t boot_magic) {
    display_init();     // Initialize display driver

    // * Check multiboot magic number
    if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC) { kernel_panic("Invalid multiboot magic number"); }

    // * Check system memory map
    if (!(boot_info->flags >> 6 & 0x01)) { kernel_panic("Invalid memory map given by bootloader"); }

    // * Detect hardware and identify
    // Check the kernel base address
    if (KERNEL_BASE != (size_t)&kernel_start) { kernel_panic("Invalid kernel base address"); }
    // Calculate kernel size
    kernel_PhysicalSize = (size_t)&kernel_end - (size_t)&kernel_start;
    // Get physical memory size from bootloader
    kernel_MemoryLowerSize = boot_info->mem_lower * 1024;   // Get lower (Below 1MB) memory size
    kernel_MemoryUpperSize = boot_info->mem_upper * 1024;   // Get upper (Above 1MB) memory size
    // Get boot device from bootloader
    if (boot_info->flags & (1 << 4)) {                      // Check boot device flag
        kernel_BootDevice = boot_info->boot_device;         // Get boot device if boot device flag set
    } else {
        kernel_BootDevice = 0;                              // Else set boot device invalid
    }

    // * Print information about hardware
    printf("Kernel size: %dMB\n", kernel_size() / 1024 / 1024);     // Print kernel size
    printf("Memory size: %dMB\n", kernel_memsize() / 1024 / 1024);  // Print physical memory size
    printf("Boot device: %d (%s)\n",                                // Print kernel boot device
        kernel_bootdev(), kernel_BootDeviceList[kernel_bootdev()]);
    printf(
        "Time and date: %d:%d:%d %d/%d/%d\n",                       // Print date and time
        date().hour, date().min, date().sec,                        // Print time
        date().day, date().mon, date().year                         // Print date
    );
    putchar('\n');

    // * Scan for available memory field
    puts("---- Scanning Memory ----\n");
    int fieldFound = false;     // Variable for check available memory field found or not
    uint64_t fieldSize;         // Variable for get available field size
    // Check for memory fields
    for (int i = 0; i < boot_info->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        // System memory map from bootloader
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (boot_info->mmap_addr + i);
        // Print current memory field
        printf("0x%x", mmmt->addr); 
        if (mmmt->len/1024/1024) {
            printf(", %dMB: ", mmmt->len/1024/1024);
        } else { printf(", %dKB: ", mmmt->len/1024); }
        // Check is available for kernel or not
        if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (mmmt->addr != KERNEL_BASE || mmmt->len < kernel_size()) {
                printf("Available but small\n");    // Available but its small for kernel
            } else {
                printf("Available, choosing this field.\n");        // Available
                fieldSize = mmmt->len; fieldFound = true; break;    // Use this field
            }
        } else { printf("Unavailable\n"); }     // Not available for use
    } if (!fieldFound) { kernel_panic("No available memory fields"); }  // Halt the system if no memory field for use
    puts("---- Scanning Ended ----\n");
    putchar('\n');

    // * Initialize kernel
    puts("---- Initializing Kernel ----\n");
    puts("Initializing Protected Mode..."); protect_init(); puts(" Success.\n");
    puts("Initializing Interrupt Manager..."); interrupts_init(); puts(" Success.\n");
    puts("Initializing Memory Manager..."); memory_init(fieldSize - kernel_size()); puts(" Success.\n");
    puts("Initializing Process Manager..."); process_init(); puts(" Success.\n");
    puts("Initializing RAM file system..."); ramfs_init(); puts(" Success.\n");
    puts("Initializing System Call Manager..."); syscall_init(); puts(" Success.\n");
    puts("---- Initializing Ended ----\n");
    putchar('\n');
    
    // * Initialize device files
    if (ramfs_createDir("/dev/") != RAMFS_STATUS_SUCCESS) { kernel_panic("Unable to create directory /dev/"); }
    void* zero = malloc(MEMORY_BLOCKSIZE); if (zero == NULL) { kernel_panic("Unable to allocate memory"); }
    fill(zero, 0, MEMORY_BLOCKSIZE);
    if (ramfs_writeFile("/dev/keyboard", MEMORY_BLOCKSIZE, zero) != RAMFS_STATUS_SUCCESS)
        { kernel_panic("Unable to create device /dev/keyboard"); }
    free(zero);

    // Print kernel boot success message and build version
    printf("Deputy Kernel Build %d, booted successfully.\n", KERNEL_BUILD);
    
    // spawn("kernel_main", kernel_main);
    // spawn("test1", test1);
    // spawn("test2", test2);
    // spawn("test3", test3);
    // yield();
    // asm volatile ("jmp *%0" : : "r"((void*)0xFFFFFFFFFFFFFC18));

    // char* progname = "test";
    // printf("0x%x\n", (size_t)&&label);
    // label:
    // asm volatile (
    //     "mov $2, %%eax\t\n"
    //     "mov %0, %%ebx\t\n"
    //     "mov %1, %%ecx\t\n"
    //     "int $0x80"
    //     :
    //     : "r"(progname), "r"(test)
    //     : "eax", "ebx", "ecx"
    // ); yield();
    // write(STDOUT, "Merhaba, dunya!\n", length("Merhaba, dunya!\n"));
    // asm volatile (
    //     "mov $4, %%eax\t\n"
    //     "mov $1, %%ebx\t\n"
    //     "mov %0, %%ecx\t\n"
    //     "mov %1, %%edx\t\n"
    //     "int $0x80"
    //     :
    //     : "r"("Nasilsiniz?\n"), "r"(length("Nasilsiniz?\n"))
    //     : "eax", "ebx", "ecx", "edx"
    // );

    spawn("testshell", testshell);
    spawn("keyboard_process", keyboard_process);
    yield();

    kernel_main();  // Switch to kernel main
}