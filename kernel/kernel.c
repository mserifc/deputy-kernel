#include "kernel.h"

#include "multiboot.h"

#include "hardware/port.h"
#include "hardware/protect.h"
#include "hardware/interrupts.h"
#include "hardware/device.h"

#include "drivers/display.h"
#include "drivers/keyboard.h"
#include "drivers/usb.h"

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

// CPU information table
kernel_CPUInfo_t kernel_CPUInfo;

// Kernel panic log
char kernel_PanicLog[256];

// System hostname buffer
char kernel_Hostname[KERNEL_HOSTNAMELEN + 1];

// Home path for built-in kernel shell
char* kernel_HomePath = "/";

// Working path buffer for built-in kernel shell
char kernel_WorkingPath[RAMFS_MAX_PATH_LENGTH];

// Short working path buffer for built-in kernel shell
char kernel_ShortWorkingPath[RAMFS_MAX_PATH_LENGTH];

// Command recursion level
int kernel_CommandRecursionLevel = 0;

// Pointer of command to execute by sudoer
char* kernel_SudoerCommand = NULL; int kernel_SudoerReturn = 0;
bool kernel_SudoPassed = false;

// Function for get kernel size
size_t kernel_size() { return kernel_PhysicalSize; }

// Function for get system memory size
size_t kernel_memsize() { return kernel_MemoryLowerSize + kernel_MemoryUpperSize; }

// Function for get kernel boot device
uint32_t kernel_bootdev() { return kernel_BootDevice; }

// Panic function for handling kernel panic situations
void kernel_panic(void) {
    printf("Kernel panic: %s", kernel_PanicLog);
    if (KERNEL_GRAPHICMODE) {
        extern unsigned char font8x16[][16];
        extern char display_TextMemory[DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT];
        extern uint8_t display_VideoMemory[DISPLAY_GUIWIDTH * DISPLAY_GUIHEIGHT];
        uint8_t* vram = (uint8_t*)DISPLAY_GUIVIDEOMEMORY;
        for (int i = 0; i < DISPLAY_CLIWIDTH * DISPLAY_CLIHEIGHT; ++i) {
            int x = (i % DISPLAY_CLIWIDTH) * DISPLAY_FONTWIDTH;
            int y = (i / DISPLAY_CLIWIDTH) * DISPLAY_FONTHEIGHT;
            for (int row = 0; row < DISPLAY_FONTHEIGHT; ++row) {
                char row_bits = font8x16[(int)display_TextMemory[i]][row];
                for (int col = 0; col < DISPLAY_FONTWIDTH; ++col) {
                    int bit = (row_bits >> DISPLAY_FONTWIDTH - 1 - col) & 1;
                    uint32_t color = bit ? 0x07 : 0x00;
                    display_VideoMemory[((y + row) * DISPLAY_GUIWIDTH) + x + col] = color;
                }
            }
        }
        for (int plane = 0; plane < 4; ++plane) {
            port_outb(0x3C4, 0x02);
            port_outb(0x3C5, 1 << plane);
            for (int y = 0; y < DISPLAY_GUIHEIGHT; ++y) {
                for (int byte = 0; byte < (DISPLAY_GUIWIDTH / 8); ++byte) {
                    uint8_t out = 0;
                    for (int bit = 0; bit < 8; ++bit) {
                        int x = byte * 8 + bit;
                        out |= ((display_VideoMemory[y * DISPLAY_GUIWIDTH + x] >> plane) & 1) << (7 - bit);
                    } vram[y * (DISPLAY_GUIWIDTH / 8) + byte] = out;
                }
            }
        }
    } while (1) { asm volatile ("cli\n hlt"); }
}

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
        sleep(5);
        yield();
        printf("Returned to example task 2\n");
    }
}

void test3() {
    while (true) {
        printf("Example task 3 in focus...\n");
        sleep(10);
        yield();
        printf("Returned to example task 3\n");
    }
}

void testshell() {
    while (true) {
        char* input = (char*)malloc(UTILS_PROMPTBUFFER_LENGTH);
        fill(input, 0, UTILS_PROMPTBUFFER_LENGTH);
        uint32_t written = read(STDIN, input, KERNEL_TEXTEDIT_BUFFSIZE);
        if (written != 0) { puts(input); }
        yield();
    }
}

void kernel_cpuid(
    uint32_t leaf, uint32_t subleaf,
    uint32_t* eax, uint32_t* ebx,
    uint32_t* ecx, uint32_t* edx
) {
    asm volatile (
        "cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "a" (leaf), "c" (subleaf)
    );
}

void sentry() {
    while (true) {
        if (kernel_SudoPassed) { sleep(300); kernel_SudoPassed = false; } yield();
    }
}

void sudoer() {
    while (true) {
        if (kernel_SudoerCommand != NULL && length(kernel_SudoerCommand) > 0 &&
            compare(split(kernel_SudoerCommand, ' ')->v[0], "su") != 0) {
            kernel_SudoerReturn = kernel_commandHandler(kernel_WorkingPath, kernel_SudoerCommand);
            --kernel_CommandRecursionLevel;
        } kernel_SudoerCommand = NULL; yield();
    }
}

void shell() {
    extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
    if (length(multitask_UserList[getuid()].home) > 0) {
        fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH);
        copy(kernel_WorkingPath, multitask_UserList[getuid()].home);
    } start: while (true) {
        if (ncompare(kernel_WorkingPath, multitask_UserList[getuid()].home, 
            length(multitask_UserList[getuid()].home)) == 0 &&
            length(multitask_UserList[getuid()].home) > 0) {
            fill(kernel_ShortWorkingPath, 0, RAMFS_MAX_PATH_LENGTH); kernel_ShortWorkingPath[0] = '~';
            copy(&kernel_ShortWorkingPath[1], &kernel_WorkingPath[length(multitask_UserList[getuid()].home) - 1]);
        } else { ncopy(kernel_ShortWorkingPath, kernel_WorkingPath, RAMFS_MAX_PATH_LENGTH); }
        printf("%s@%s %s ", multitask_UserList[getuid()].name, kernel_Hostname, kernel_ShortWorkingPath);
        char* cmd = prompt(getuid() ? "$ " : "# "); putchar('\n');
        if (split(cmd, ' ')->v[0] && compare(split(cmd, ' ')->v[0], "exit") == 0) { /* Process completed */ break; }
        if (kernel_commandHandler(kernel_WorkingPath, cmd) != EXIT_SUCCESS) { /* Handle error */ }
        --kernel_CommandRecursionLevel; yield();
    } /* Restart shell */ goto start;
}

void idle() { while(true) { yield(); } }

void init() {
    spawn("display", display_process);
    spawn("keyboard", keyboard_process);
    spawn("usbd", usb_process);
    spawn("sentry", sentry);
    spawn("sudoer", sudoer);
    spawn("shell", shell);
    while (true) { yield(); }
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
                        if (split(input, ' ')->c == 1) {
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
                                    // Do nothing
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
    if (kernel_CommandRecursionLevel < KERNEL_CMDRECURSIONLIMIT) { ++kernel_CommandRecursionLevel; }
    else { printf("Recursion limit exceeded\n"); return EXIT_FAILURE; }
    if (length(cmd) <= 0) { return 0; }
    tokens_t* arg = split(cmd, ' ');
    if (arg->c > 0) {
        /* --> */ if (arg->v[0] && compare(arg->v[0], "echo") == 0) {
            for (int i = 1; i < arg->c; ++i) {
                printf(arg->v[i]);
                if (i+1 != arg->c) { putchar(' '); }
            } putchar('\n'); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "clear") == 0) {
            display_clear(); putcursor(0); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "pwd") == 0) {
            printf("%s\n", path); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "cd") == 0) {
            if (arg->c < 2) {
                uid_t user = getuid();
                if (user >= 0 && user < MULTITASK_USER_COUNTMAX) {
                    extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
                    if (length(multitask_UserList[user].home) > 0) {
                        if (ramfs_stat(multitask_UserList[user].home) != NULL) {
                            fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH);
                            copy(kernel_WorkingPath, multitask_UserList[user].home);
                            return EXIT_SUCCESS;
                        } else { printf("Home directory not exists for current user\n"); return EXIT_FAILURE; }
                    } else { printf("No home directory set for current user\n"); return EXIT_FAILURE; }
                } else {
                    fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH);
                    copy(kernel_WorkingPath, kernel_HomePath);
                    return EXIT_SUCCESS;
                }
            } if (arg->v[1] && compare(arg->v[1], ".") == 0) {
                return EXIT_SUCCESS;
            } else if (arg->v[1] && compare(arg->v[1], "..") == 0) {
                tokens_t* path = split(kernel_WorkingPath, '/');
                int ptr = 0; for (int i = 0; i < path->c - 1; ++i) { ptr += length(path->v[i]) + 1; }
                fill(&kernel_WorkingPath[ptr+1], 0, length(&kernel_WorkingPath[ptr]));
                return EXIT_SUCCESS;
            }
            if (length(arg->v[1]) >= RAMFS_MAX_PATH_LENGTH) { printf("Path too long: %s\n", arg->v[1]); return EXIT_FAILURE; }
            if (arg->v[1][0] != '/') {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg->v[1][length(arg->v[1]) - 1] == '/') {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg->v[1]);
                } else { snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg->v[1]); }
                if (ramfs_stat(buffer) == NULL)
                    { printf("No such directory: %s\n", arg->v[1]); return EXIT_FAILURE; }
                fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, buffer);
                free(buffer); return EXIT_SUCCESS;
            }
            if (arg->v[1][length(arg->v[1]) - 1] != '/') {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s/", arg->v[1]);
                if (ramfs_stat(buffer) == NULL)
                    { printf("No such directory: %s\n", arg->v[1]); return EXIT_FAILURE; }
                fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, buffer);
                free(buffer); return EXIT_SUCCESS;
            }
            if (ramfs_stat(arg->v[1]) == NULL) { printf("No such directory: %s\n", arg->v[1]); return EXIT_FAILURE; }
            fill(kernel_WorkingPath, 0, RAMFS_MAX_PATH_LENGTH); copy(kernel_WorkingPath, arg->v[1]); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "ls") == 0) {
            char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
            if (arg->c < 2) { copy(buffer, path); } else {
                if (arg->v[1][0] == '/') { copy(arg->v[1], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg->v[1]);
                }
            }
            if (ramfs_stat(buffer) == NULL) { printf("No such directory: %s\n", buffer); free(buffer); return EXIT_FAILURE; }
            int* ents = ramfs_readDir(buffer);
            if (ents == NULL) { printf("Unable to read directory: %s\n", buffer); return EXIT_FAILURE; }
            for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT && ents[i] != RAMFS_DIRENTEND; ++i) {
                ramfs_Entry_t* ent = ramfs_dirent(ents[i]);
                if (
                    (ent->name && compare(ent->name, buffer) == 0) ||
                    split(ent->name, '/')->c > split(buffer, '/')->c + 1
                ) { continue; }
                if (ent) { printf("%s\n", &ent->name[length(buffer)]); }
            } free(buffer); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "cat") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            if (arg->v[1][0] == '/') {
                if (ramfs_stat(arg->v[1]) == NULL || ramfs_stat(arg->v[1])->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg->v[1]); return EXIT_FAILURE; }
                char* data = ramfs_readFile(arg->v[1]);
                for (int i = 0; i < ramfs_stat(arg->v[1])->size; ++i) { if (data[i] != '\0') { putchar(data[i]); } }
                putchar('\n'); return EXIT_SUCCESS;
            } else {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg->v[1]);
                if (ramfs_stat(buffer) == NULL || ramfs_stat(buffer)->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg->v[1]); return EXIT_FAILURE; }
                char* data = ramfs_readFile(buffer);
                for (int i = 0; i < ramfs_stat(buffer)->size; ++i) { if (data[i] != '\0') { putchar(data[i]); } }
                putchar('\n'); free(buffer); return EXIT_SUCCESS;
            }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "mkdir") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
            if (arg->v[1][0] == '/') { copy(arg->v[1], buffer); } else {
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg->v[1]);
            } if (ramfs_stat(buffer) == NULL) {
                if (ramfs_createDir(buffer) != RAMFS_STATUS_SUCCESS)
                    { printf("Unable to make: %s\n", buffer); free(buffer); return EXIT_FAILURE; }
            } else { printf("Already exists: %s\n", buffer); free(buffer); return EXIT_FAILURE; }
            free(buffer); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "touch") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
            if (arg->v[1][0] == '/') { copy(arg->v[1], buffer); } else {
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg->v[1]);
            }
            if (ramfs_stat(buffer) != NULL) {
                ramfs_stat(buffer)->mtime = date();
                ramfs_stat(buffer)->atime = date();
            } else {
                if (ramfs_writeFile(buffer, 1, " ") != RAMFS_STATUS_SUCCESS)
                    { printf("Unable to write %s\n", buffer); free(buffer); return EXIT_FAILURE; }
            } free(buffer); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "rmdir") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            for (int i = 1; i < arg->c; ++i) {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg->v[i][0] == '/') { copy(arg->v[i], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s/", kernel_WorkingPath, arg->v[i]);
                }
                if (ramfs_stat(buffer) != NULL) {
                    if (ramfs_stat(buffer)->type != RAMFS_TYPE_DIR) { printf("Not a directory: %s\n", buffer); }
                    bool empty = true;
                    int* ents = ramfs_readDir(buffer);
                    if (ents == NULL)
                        { printf("Unable to read directory: %s\n", buffer); free(buffer); return EXIT_FAILURE; }
                    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT && ents[i] != RAMFS_DIRENTEND; ++i) {
                        ramfs_Entry_t* ent = ramfs_dirent(ents[i]);
                        if ((ent->name && compare(ent->name, buffer) == 0) ||
                            split(ent->name, '/')->c > split(buffer, '/')->c + 1
                        ) { continue; } if (ent) { empty = false; }
                    } if (!empty) { printf("Directory not empty\n"); free(buffer); return EXIT_FAILURE; }
                    if (ramfs_remove(buffer) != RAMFS_STATUS_SUCCESS) { printf("Unable to remove %s\n", buffer); }
                } else { printf("No such directory: %s\n", buffer); } free(buffer);
            } return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "rm") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            for (int i = 1; i < arg->c; ++i) {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                if (arg->v[i][0] == '/') { copy(arg->v[i], buffer); } else {
                    snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg->v[i]);
                }
                if (ramfs_stat(buffer) != NULL) {
                    if (ramfs_stat(buffer)->type == RAMFS_TYPE_DIR) { printf("It's a directory: %s\n", buffer); }
                    if (ramfs_remove(buffer) != RAMFS_STATUS_SUCCESS) { printf("Unable to remove %s\n", buffer); }
                } else { printf("No such file: %s\n", buffer); } free(buffer);
            } return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "edit") == 0) {
            if (arg->c < 2) { return kernel_textEditor(NULL); }
            if (arg->v[1][0] == '/') {
                if (ramfs_stat(arg->v[1]) == NULL || ramfs_stat(arg->v[1])->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg->v[1]); return EXIT_FAILURE; }
                return kernel_textEditor(arg->v[1]);
            } else {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg->v[1]);
                if (ramfs_stat(buffer) == NULL || ramfs_stat(buffer)->type != RAMFS_TYPE_FILE)
                    { printf("No such file: %s\n", arg->v[1]); return EXIT_FAILURE; }
                return kernel_textEditor(buffer);
            }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "useradd") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            bool homedir = false; if (arg->v[1][0] == '-') {
                if (length(arg->v[1]) < 2) { printf("Missing parameters\n"); return EXIT_USAGE_ERROR; }
                if (arg->v[1][1] == 'm') { homedir = true; }
                else { printf("Unknown parameter: %s\n", arg->v[1]); return EXIT_USAGE_ERROR; }
            } if (homedir) {
                if (uadd(arg->v[2]) != 0) { printf("Unable to add user: %s\n", arg->v[2]); return EXIT_FAILURE; }
                extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
                for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                    if (!multitask_UserList[i].active) { continue; }
                    if (compare(arg->v[2], multitask_UserList[i].name) != 0) { continue; }
                    snprintf(multitask_UserList[i].home, MULTITASK_USER_HOMEMAX,
                        "/home/%s/", multitask_UserList[i].name);
                    ramfs_createDir(multitask_UserList[i].home);
                    if (ramfs_stat(multitask_UserList[i].home) == NULL) {
                        fill(multitask_UserList[i].home, 0, MULTITASK_USER_HOMEMAX); break;
                    } return EXIT_SUCCESS;
                } printf("Unable to create home directory\n"); return EXIT_FAILURE;
            } else { if (uadd(arg->v[1]) != 0) { printf("Unable to add user\n"); return EXIT_FAILURE; } }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "userdel") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            bool rmhome = false; if (arg->v[1][0] == '-') {
                if (length(arg->v[1]) < 2) { printf("Missing parameters\n"); return EXIT_USAGE_ERROR; }
                if (arg->v[1][1] == 'r') { rmhome = true; }
                else { printf("Unknown parameter: %s\n", arg->v[1]); return EXIT_USAGE_ERROR; }
            } if (rmhome) {
                extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
                for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                    if (!multitask_UserList[i].active) { continue; }
                    if (compare(arg->v[2], multitask_UserList[i].name) != 0) { continue; }
                    if (length(multitask_UserList[i].home) <= 0) { break; }
                    if (ramfs_bulkRemove(multitask_UserList[i].home) != RAMFS_STATUS_SUCCESS)
                        { printf("Unable to remove directory: %s\n", multitask_UserList[i].home); break; }
                }
            } if (udel(rmhome ? arg->v[2] : arg->v[1]) != 0)
                { printf("Unable to delete user: %s\n", rmhome ? arg->v[2] : arg->v[1]); return EXIT_FAILURE; }
            return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "usermod") == 0) {
            if (arg->c < 4) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            if (compare(arg->v[3], "root") == 0 ||
                compare(arg->v[3], "daemon") == 0 ||
                compare(arg->v[3], "nobody") == 0)
                { printf("Operation not permitted\n"); return EXIT_PERMISSION_DENIED; }
            extern pid_t multitask_Focus; if (multitask_Focus == 0 || getuid() == MULTITASK_USER_ROOT) {
                extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
                extern char multitask_GroupList[MULTITASK_USER_COUNTMAX][MULTITASK_USER_NAMELEN + 1];
                if (compare(arg->v[1], "-u") == 0) {
                    int new_uid = convert_atoi(arg->v[2]);
                    if (new_uid < 0 || new_uid >= MULTITASK_USER_COUNTMAX)
                        { printf("Invalid user ID: %d\n", new_uid); return EXIT_USAGE_ERROR; }
                    if (multitask_UserList[new_uid].active)
                        { printf("User ID %d is already in use\n", new_uid); return EXIT_FAILURE; }
                    uid_t old_uid = -1; for (pid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (!multitask_UserList[i].active) { continue; }
                        if (compare(multitask_UserList[i].name, arg->v[3]) == 0) { old_uid = i; break; }
                    } if (old_uid == -1) { printf("User %s not found\n", arg->v[3]); return EXIT_FAILURE; }
                    ncopy(&multitask_UserList[new_uid], &multitask_UserList[old_uid], sizeof(multitask_User_t));
                    fill(&multitask_UserList[old_uid], 0, sizeof(multitask_User_t));
                    for (gid_t i = 0; i < MULTITASK_USER_GROUPMAX; ++i) {
                        multitask_UserList[old_uid].group[i] = MULTITASK_USER_NULL;
                    } return EXIT_SUCCESS;
                } else if (compare(arg->v[1], "-l") == 0) {
                    if (length(arg->v[2]) > MULTITASK_USER_NAMELEN)
                        { printf("Username too long\n"); return EXIT_USAGE_ERROR; }
                    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (!multitask_UserList[i].active) { continue; }
                        if (compare(multitask_UserList[i].name, arg->v[3]) == 0) {
                            fill(multitask_UserList[i].name, 0, MULTITASK_USER_NAMELEN);
                            copy(multitask_UserList[i].name, arg->v[2]); return EXIT_SUCCESS;
                        }
                    } printf("User %s not found\n", arg->v[3]); return EXIT_FAILURE;
                } else if (compare(arg->v[1], "-d") == 0) {
                    if (length(arg->v[2]) > MULTITASK_USER_HOMEMAX)
                        { printf("Home directory path too long\n"); return EXIT_USAGE_ERROR; }
                    if (ramfs_stat(arg->v[2]) == NULL)
                        { printf("No such file or directory: %s\n", arg->v[2]); return EXIT_FAILURE; }
                    if (ramfs_stat(arg->v[2])->type != RAMFS_TYPE_DIR)
                        { printf("Not a directory: %s\n", arg->v[2]); return EXIT_FAILURE; }
                    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (!multitask_UserList[i].active) { continue; }
                        if (compare(multitask_UserList[i].name, arg->v[3]) == 0) {
                            fill(multitask_UserList[i].home, 0, MULTITASK_USER_HOMEMAX);
                            copy(multitask_UserList[i].home, arg->v[2]); return EXIT_SUCCESS;
                        }
                    } printf("User %s not found\n", arg->v[3]); return EXIT_FAILURE;
                } else if (compare(arg->v[1], "-g") == 0) {
                    if (length(arg->v[2]) > MULTITASK_USER_NAMELEN)
                        { printf("Group name too long\n"); return EXIT_USAGE_ERROR; }
                    gid_t gid = -1; for (gid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (length(multitask_GroupList[i]) <= 0) { continue; }
                        if (compare(multitask_GroupList[i], arg->v[2]) == 0) { gid = i; break; }
                    } if (gid == -1) { printf("Group %s not found\n", arg->v[2]); return EXIT_FAILURE; }
                    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (!multitask_UserList[i].active) { continue; }
                        if (compare(multitask_UserList[i].name, arg->v[3]) == 0) {
                            for (gid_t j = 0; j < MULTITASK_USER_GROUPMAX; ++j) {
                                multitask_UserList[i].group[j] = MULTITASK_USER_NULL;
                            } multitask_UserList[i].group[0] = gid; return EXIT_SUCCESS;
                        }
                    } printf("User %s not found\n", arg->v[3]); return EXIT_FAILURE;
                } else if (compare(arg->v[1], "-G") == 0) {
                    if (length(arg->v[2]) > MULTITASK_USER_NAMELEN)
                        { printf("Group name too long\n"); return EXIT_USAGE_ERROR; }
                    gid_t gid = -1; for (gid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (length(multitask_GroupList[i]) <= 0) { continue; }
                        if (compare(multitask_GroupList[i], arg->v[2]) == 0) { gid = i; break; }
                    } if (gid == -1) { printf("Group %s not found\n", arg->v[2]); return EXIT_FAILURE; }
                    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (!multitask_UserList[i].active) { continue; }
                        if (compare(multitask_UserList[i].name, arg->v[3]) == 0) {
                            for (gid_t j = 1; j < MULTITASK_USER_GROUPMAX; ++j) {
                                multitask_UserList[i].group[j] = MULTITASK_USER_NULL;
                            } if (multitask_UserList[i].group[0] != gid) { multitask_UserList[i].group[1] = gid; }
                            return EXIT_SUCCESS;
                        }
                    } printf("User %s not found\n", arg->v[3]); return EXIT_FAILURE;
                } else if (compare(arg->v[1], "-aG") == 0) {
                    if (length(arg->v[2]) > MULTITASK_USER_NAMELEN)
                        { printf("Group name too long\n"); return EXIT_USAGE_ERROR; }
                    gid_t gid = -1; for (gid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (length(multitask_GroupList[i]) <= 0) { continue; }
                        if (compare(multitask_GroupList[i], arg->v[2]) == 0) { gid = i; break; }
                    } if (gid == -1) { printf("Group %s not found\n", arg->v[2]); return EXIT_FAILURE; }
                    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                        if (!multitask_UserList[i].active) { continue; }
                        if (compare(multitask_UserList[i].name, arg->v[3]) == 0) {
                            int gcount = 0; for (gid_t j = 0; j < MULTITASK_USER_GROUPMAX; ++j) { gcount = j;
                                if (multitask_UserList[i].group[j] == MULTITASK_USER_NULL) { break;}
                                if (multitask_UserList[i].group[j] == gid) {
                                    printf("User %s already in group %s\n",
                                        multitask_UserList[i].name, multitask_GroupList[j]);
                                    return EXIT_FAILURE;
                                }
                            } if (gcount >= MULTITASK_USER_GROUPMAX)
                                { printf("Group limit has been exceeded\n"); return EXIT_FAILURE; }
                            multitask_UserList[i].group[gcount] = gid; return EXIT_SUCCESS;
                        }
                    } printf("User %s not found\n", arg->v[3]); return EXIT_FAILURE;
                } else { printf("Unknown option: %s\n", arg->v[1]); return EXIT_USAGE_ERROR; }
            } else { printf("Permission denied\n"); return EXIT_PERMISSION_DENIED; }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "passwd") == 0) {
            if (arg->c < 2) {
                extern pid_t multitask_Focus;
                extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
                if (getuid() == -1) { printf("No users active in kernel mode\n"); return EXIT_FAILURE; }
                if (getuid() != MULTITASK_USER_ROOT) {
                    char* oldpass = (char*)malloc(UTILS_PROMPTBUFFER_LENGTH);
                    if (oldpass == NULL) { printf("Out of memory\n"); return EXIT_FAILURE; }
                    puts("Current "); copy(oldpass, prompt("Password: ")); putchar('\n');
                    puts("New "); if (upwd(multitask_UserList[getuid()].name, oldpass, prompt("Password: ")) != 0)
                        { printf("\nUnable to change password of %s\n", multitask_UserList[getuid()].name); }
                    free(oldpass); putchar('\n'); return EXIT_SUCCESS;
                } else {
                    puts("New "); if (upwd(multitask_UserList[getuid()].name, NULL, prompt("Password: ")) != 0)
                        { printf("\nUnable to change password of %s", multitask_UserList[getuid()].name); }
                } putchar('\n'); return EXIT_SUCCESS;
            } else {
                extern pid_t multitask_Focus;
                extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
                for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
                    if (!multitask_UserList[i].active) { continue; }
                    if (compare(arg->v[1], multitask_UserList[i].name) != 0) { continue; }
                    if (multitask_Focus == 0 || getuid() == MULTITASK_USER_ROOT) {
                        puts("New "); if (upwd(multitask_UserList[i].name, NULL, prompt("Password: ")) != 0)
                            { printf("\nUnable to change password of %s", multitask_UserList[i].name); }
                        putchar('\n'); return EXIT_SUCCESS;
                    } else if (getuid() == i) {
                        char* oldpass = (char*)malloc(UTILS_PROMPTBUFFER_LENGTH);
                        if (oldpass == NULL) { printf("Out of memory\n"); return EXIT_FAILURE; }
                        puts("Current "); copy(oldpass, prompt("Password: ")); putchar('\n');
                        puts("New "); if (upwd(multitask_UserList[i].name, oldpass, prompt("Password: ")) != 0)
                            { printf("\nUnable to change password of %s", multitask_UserList[i].name); }
                        free(oldpass); putchar('\n'); return EXIT_SUCCESS;
                    } else { printf("Permission denied\n"); return EXIT_FAILURE; }
                } printf("User not found\n"); return EXIT_FAILURE;
            }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "run") == 0) {
            if (arg->c < 2) { printf("Specific file required to run\n"); return EXIT_USAGE_ERROR; }
            ramfs_Entry_t* file;
            if (arg->v[1][0] == '/') {
                file = ramfs_stat(arg->v[1]);
                if (file == NULL) { printf("No such file: %s\n", arg->v[1]); return EXIT_FAILURE; }
            } else {
                char* buffer = (char*)malloc(RAMFS_MAX_PATH_LENGTH); if (buffer == NULL) { return EXIT_FAILURE; }
                snprintf(buffer, RAMFS_MAX_PATH_LENGTH, "%s%s", kernel_WorkingPath, arg->v[1]);
                file = ramfs_stat(buffer); free(buffer);
                if (file == NULL) { printf("No such file: %s\n", arg->v[1]); return EXIT_FAILURE; }
            } char* script = ramfs_readFile(file->name);
            if (script == NULL) { printf("Unable to read file: %s\n", file->name); return EXIT_FAILURE; }
            int ptr = 0; char* cmd = (char*)malloc(MEMORY_BLOCKSIZE);
            if (cmd == NULL) { printf("Out of memory\n"); return EXIT_FAILURE; } fill(cmd, 0, file->size);
            for (size_t i = 0; i <= file->size; ++i) {
                if (script[i] == '\n' || script[i] == '\0' || ptr >= MEMORY_BLOCKSIZE || i >= file->size) {
                    cmd[ptr] = '\0';
                    kernel_commandHandler(kernel_WorkingPath, cmd); --kernel_CommandRecursionLevel;
                    fill(cmd, 0, file->size); ptr = 0;
                } else { cmd[ptr] = script[i]; ++ptr; }
            } free(cmd); return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "kill") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            pid_t pid = convert_atoi(arg->v[1]);
            if (pid < 1 || pid >= MULTITASK_ENTCOUNT)
                { printf("Invalid process ID: %d\n", pid); return EXIT_USAGE_ERROR; }
            extern multitask_Process_t* multitask_ProcessTable;
            if (!multitask_ProcessTable[pid].active)
                { printf("(%d) - No such process\n", pid); return EXIT_FAILURE; }
            if (kill(pid) != 0) { printf("(%d) - Operation not permitted\n", pid); return EXIT_PERMISSION_DENIED; }
            return EXIT_SUCCESS;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "pkill") == 0) {
            if (arg->c < 2) { printf("Too few arguments\n"); return EXIT_USAGE_ERROR; }
            extern multitask_Process_t* multitask_ProcessTable;
            bool err = false; for (pid_t i = 1; i < MULTITASK_ENTCOUNT; ++i) {
                if (!multitask_ProcessTable[i].active) { continue; }
                if (compare(multitask_ProcessTable[i].name, arg->v[1]) == 0) {
                    if (kill(i) != 0) { printf("(%d) - Operation not permitted\n", i); err = true; }
                }
            } if (!err) { return EXIT_SUCCESS; } else { return EXIT_FAILURE; }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "ps") == 0) {
            char* report = multitask_report(); if (report == NULL) {
                printf("Unable to get list of processes\n"); return EXIT_FAILURE;
            } puts("USER\tPID\tPPID\tNAME\n"); puts(report);
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "mem") == 0) {
            char* report = memory_report(); if (report == NULL) {
                printf("Unable to get list of processes\n"); return EXIT_FAILURE;
            } printf("%s\n", report);
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "lscpu") == 0) {
            printf("Vendor: %s\n", kernel_CPUInfo.vendor);
            printf("Brand: %s\n", kernel_CPUInfo.brand);
            printf("Frequency: %dMHz\n", kernel_CPUInfo.frequency/1024/1024);
            printf("Cores\tThreads\tSSE\tAVX\tVT-x\tAES\tx64\n");
            printf("%d\t%d\t%s\t%s\t%s\t%s\t%s\n",
                kernel_CPUInfo.cores,
                kernel_CPUInfo.threads,
                kernel_CPUInfo.has_sse ? "Yes" : "No",
                kernel_CPUInfo.has_avx ? "Yes" : "No",
                kernel_CPUInfo.has_vtx ? "Yes" : "No",
                kernel_CPUInfo.has_aes ? "Yes" : "No",
                kernel_CPUInfo.has_x64 ? "Yes" : "No");
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "su") == 0) {
            if (arg->c < 2) {
                if (getuid() != 0) {
                    char* entry = prompt("Password: ");
                    putchar('\n'); sleep(1);
                    int n = uswi("root", entry);
                    if (n != 0) {
                        printf("Sorry\n"); return EXIT_FAILURE;
                    } kernel_SudoPassed = false; return EXIT_SUCCESS;
                } else { if (uswi("root", NULL) != 0) { printf("Unknown error\n"); return EXIT_FAILURE; } }
            } else {
                if (compare(arg->v[1], "daemon") == 0 || compare(arg->v[1], "nobody") == 0)
                    { printf("This account is currently not available\n"); return EXIT_PERMISSION_DENIED; }
                if (getuid() != 0) {
                    char* entry = prompt("Password: ");
                    putchar('\n'); sleep(1);
                    int n = uswi(arg->v[1], entry);
                    if (n != 0) {
                        printf("Sorry\n"); return EXIT_FAILURE;
                    } kernel_SudoPassed = false; return EXIT_SUCCESS;
                } else { if (uswi(arg->v[1], NULL) != 0) { printf("Unknown login\n"); return EXIT_FAILURE; } }
            }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "sudo") == 0) {
            if (arg->c < 2) { printf("Usage: sudo [command [arg ...]]\n"); return EXIT_USAGE_ERROR; }
            uid_t uid = getuid(); extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
            if (uid != MULTITASK_USER_ROOT) {
                bool access = false; for (gid_t i = 0; i < MULTITASK_USER_GROUPMAX; ++i) {
                    if (multitask_UserList[uid].group[i] == MULTITASK_USER_SUDO) { access = true; break; }
                } if (!access) {
                    printf("%s is not authorized to use sudo\n", multitask_UserList[uid].name);
                    return EXIT_PERMISSION_DENIED;
                }
                if (!kernel_SudoPassed) {
                    char* pass = prompt("Password: "); putchar('\n'); sleep(1);
                    if (multitask_UserList[uid].pass != fnv1ahash(pass, length(pass)))
                        { printf("Sorry\n"); return EXIT_FAILURE; }
                } kernel_SudoPassed = true;
            } else { uid = MULTITASK_USER_ROOT; }
            char* buffer = (char*)malloc(UTILS_PROMPTBUFFER_LENGTH);
            if (buffer == NULL) { printf("Out of memory\n"); return EXIT_FAILURE; }
            fill(buffer, 0, UTILS_PROMPTBUFFER_LENGTH); int ptr = 0;
            for (int i = 1; i < arg->c; ++i) {
                copy(&buffer[ptr], arg->v[i]); ptr += length(arg->v[i]);
                if (i+1 != arg->c) { buffer[ptr] = ' '; ++ptr; }
            } buffer[ptr] = '\0'; kernel_SudoerCommand = buffer; yield(); free(buffer); return kernel_SudoerReturn;
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "whoami") == 0) {
            if (getuid() == -1) { return EXIT_FAILURE; }
            extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
            printf("%s\n", multitask_UserList[getuid()].name);
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "uname") == 0) {
            if (arg->c < 2) { printf("%s\n", KERNEL_NAME); return EXIT_SUCCESS; } else {
                if (arg->v[1][0] != '-') { printf("Usage: uname [-snrvim]\n"); return EXIT_USAGE_ERROR; }
                if (length(arg->v[1]) < 2) { printf("Missing parameters\n"); return EXIT_USAGE_ERROR; }
                bool
                    ps = false, pn = false, pr = false,
                    pv = false, pi = false, pm = false;
                for (int i = 1; i < length(arg->v[1]); ++i) {
                    switch (arg->v[1][i]) {
                        case 'a': {
                            ps = true; pn = true; pr = true;
                            pv = true; pi = true; pm = true;
                            break;
                        }
                        case 's': { ps = true; break; }
                        case 'n': { pn = true; break; }
                        case 'r': { pr = true; break; }
                        case 'v': { pv = true; break; }
                        case 'i': { pi = true; break; }
                        case 'm': { pm = true; break; }
                    }
                } if (!ps && !pn && !pr && !pv && !pi && !pm)
                    { printf("Usage: uname [-snrvim]\n"); return EXIT_USAGE_ERROR; }
                if (ps) { printf("%s ", KERNEL_NAME); }
                if (pn) { printf("%s ", kernel_Hostname); }
                if (pr) { printf("%s ", KERNEL_RELEASE); }
                if (pv) { printf("%s ", KERNEL_VERSION); }
                if (pi) { printf("%s ", KERNEL_PLATFORM); }
                if (pm) { printf("%s", kernel_CPUInfo.has_x64 ? "x86_64" : "i386"); }
                putchar('\n'); return EXIT_SUCCESS;
            }
        } /* --> */ else if (arg->v[0] && compare(arg->v[0], "test") == 0) {
            // * Command for quickly test embedded functions before compilation
            // extern multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];
            // for (int i = 0; i < MULTITASK_USER_GROUPMAX; ++i) {
            //     printf("%d\n", multitask_UserList[2].group[i]);
            // }

            // device_t devinfo;
            // // BAR_t devbar;
            // puts("VENDOR\tDEVICE\tCMD\tSTATUS\tREV\tIF\tSUBC\tCLASS\n");
            // for (int b = 0; b < 256; ++b) {
            //     for (int s = 0; s < 32; ++s) {
            //         for (int f = 0; f < 8; ++f) {
            //             device_get(&devinfo, b, s, f);
            //             // device_getBAR(&devbar, b, s, f, 4);
            //             if (devinfo.vendor != 0xFFFF) {
            //                 printf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\n",
            //                     devinfo.vendor,
            //                     devinfo.device,
            //                     devinfo.command,
            //                     devinfo.status,
            //                     devinfo.revision,
            //                     devinfo.interface,
            //                     devinfo.subclass,
            //                     devinfo.class,
            //                     // ((device_read(b, s, f, 3) >> 16) & 0xFF) >> 7
            //                     // (size_t)devbar.addr
            //                     devinfo.port);
            //             }
            //         }
            //     }
            // }
            struct RSDP_t {
                char Signature[8];
                uint8_t Checksum;
                char OEMID[6];
                uint8_t Revision;
                uint32_t RsdtAddress;
            } __attribute__ ((packed));
            struct ACPISDTHeader {
                char Signature[4];
                uint32_t Length;
                uint8_t Revision;
                uint8_t Checksum;
                char OEMID[6];
                char OEMTableID[8];
                uint32_t OEMRevision;
                uint32_t CreatorID;
                uint32_t CreatorRevision;
            } PACKED;
            struct GenericAddressStructure {
                uint8_t AddressSpace;
                uint8_t BitWidth;
                uint8_t BitOffset;
                uint8_t AccessSize;
                uint64_t Address;
            };
            struct FADT {
                struct   ACPISDTHeader h;
                uint32_t FirmwareCtrl;
                uint32_t Dsdt;

                // field used in ACPI 1.0; no longer in use, for compatibility only
                uint8_t  Reserved;

                uint8_t  PreferredPowerManagementProfile;
                uint16_t SCI_Interrupt;
                uint32_t SMI_CommandPort;
                uint8_t  AcpiEnable;
                uint8_t  AcpiDisable;
                uint8_t  S4BIOS_REQ;
                uint8_t  PSTATE_Control;
                uint32_t PM1aEventBlock;
                uint32_t PM1bEventBlock;
                uint32_t PM1aControlBlock;
                uint32_t PM1bControlBlock;
                uint32_t PM2ControlBlock;
                uint32_t PMTimerBlock;
                uint32_t GPE0Block;
                uint32_t GPE1Block;
                uint8_t  PM1EventLength;
                uint8_t  PM1ControlLength;
                uint8_t  PM2ControlLength;
                uint8_t  PMTimerLength;
                uint8_t  GPE0Length;
                uint8_t  GPE1Length;
                uint8_t  GPE1Base;
                uint8_t  CStateControl;
                uint16_t WorstC2Latency;
                uint16_t WorstC3Latency;
                uint16_t FlushSize;
                uint16_t FlushStride;
                uint8_t  DutyOffset;
                uint8_t  DutyWidth;
                uint8_t  DayAlarm;
                uint8_t  MonthAlarm;
                uint8_t  Century;

                // reserved in ACPI 1.0; used since ACPI 2.0+
                uint16_t BootArchitectureFlags;

                uint8_t  Reserved2;
                uint32_t Flags;

                // 12 byte structure; see below for details
                struct GenericAddressStructure ResetReg;

                uint8_t  ResetValue;
                uint8_t  Reserved3[3];
            
                // 64bit pointers - Available on ACPI 2.0+
                uint64_t                X_FirmwareControl;
                uint64_t                X_Dsdt;

                struct GenericAddressStructure X_PM1aEventBlock;
                struct GenericAddressStructure X_PM1bEventBlock;
                struct GenericAddressStructure X_PM1aControlBlock;
                struct GenericAddressStructure X_PM1bControlBlock;
                struct GenericAddressStructure X_PM2ControlBlock;
                struct GenericAddressStructure X_PMTimerBlock;
                struct GenericAddressStructure X_GPE0Block;
                struct GenericAddressStructure X_GPE1Block;
            };
            struct MCFG {
                char Signature[4];
                uint32_t Length;
                uint8_t Revision;
                uint8_t Checksum;
                char OEMID[6];
                char OEMTableID[8];
                uint32_t OEMRevision;
                uint32_t CreatorID;
                uint32_t CreatorRevision;
                uint64_t Reserved;
                uint64_t pciebase;
            };
            struct DEVICE {
                uint16_t vendor;
                uint16_t device;
                uint16_t command;
                uint16_t status;
                uint8_t revision;
                uint8_t interface;
                uint8_t subclass;
                uint8_t class;
                uint8_t cachesize;
                uint8_t latencytimer;
                uint8_t headertype;
                uint8_t bist;
                uint32_t BAR0;
            };
            struct RSDP_t* rsdp;
            for (size_t i = 0x80000; i < 0xFFFFF; ++i) {
                if (ncompare((void*)i, "RSD PTR ", 6) == 0) {
                    printf("Found! Its 0x%x\n", i);
                    rsdp = (struct RSDP_t*)i;
                    printf("Signature: %s\nChecksum: 0x%x\nOEMID: %s\nRevision: %d\nRsdtAddress: 0x%x\n",
                        rsdp->Signature, rsdp->Checksum, rsdp->OEMID, rsdp->Revision, rsdp->RsdtAddress);
                    break;
                }
            } printf("Scanning ended\n");
            struct ACPISDTHeader* rsdt = (struct ACPISDTHeader*)rsdp->RsdtAddress;
            unsigned char sum = 0;
            for (int i = 0; i < rsdt->Length; i++) {
                sum += ((char *) rsdt)[i];
            } if (sum != 0) { printf("Checksum invalid!\n"); return -1; }
            else { printf("Got valid checksum\n"); }
            int count = (rsdt->Length - sizeof(rsdt)) / 4; printf("Table count: %d\n", count);
            uint32_t* othersdt = (uint32_t*)((size_t)rsdt + sizeof(struct ACPISDTHeader));
            for (int i = 0; i < count; ++i) {
                struct ACPISDTHeader* t = (struct ACPISDTHeader*)othersdt[i];
                if (ncompare(t->Signature, "MCFG", 4) == 0) {
                    printf("Found table at index %d\n", i);
                }
            } printf("Table scanning ended\n");
            struct FADT* fadt = (struct FADT*)othersdt[0];
            // port_outb(fadt->SMI_CommandPort,fadt->AcpiEnable);
            // while (port_inw(fadt->PM2ControlBlock) & 1 == 0); printf("Success\n");
            // printf("0x%x\n", port_inw(fadt->PM1aEventBlock));
            // port_outw(fadt->PM1aEventBlock + 2, (1 << 8));
            // if (port_inw(fadt->PM1aEventBlock) & (1 << 8)) {
            //     port_outw(fadt->PM1aControlBlock, 0x2000);
            // }
            struct MCFG* mcfg = (struct MCFG*)othersdt[3];
            printf("PCIe base: 0x%x\n", (size_t)mcfg->pciebase);
            struct DEVICE* dev = (struct DEVICE*)((size_t)mcfg->pciebase + ((0) << 20 | 0 << 15 | 0 << 12));
            printf("0:0:0:\n");
            puts("VENDOR\tDEVICE\tCMD\tSTATUS\tREV\tIF\tSUBC\tCLASS\n");
            printf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\n",
                dev->vendor,
                dev->device,
                dev->command,
                dev->status,
                dev->revision,
                dev->interface,
                dev->subclass,
                dev->class);
            puts("Device List:\nVENDOR\tDEVICE\tCMD\tSTATUS\tREV\tIF\tSUBC\tCLASS\n");
            for (int b = 0; b < 256; ++b) {
                for (int s = 0; s < 32; ++s) {
                    for (int f = 0; f < 8; ++f) {
                        struct DEVICE* devinfo = (struct DEVICE*)((size_t)mcfg->pciebase + ((b) << 20 | s << 15 | f << 12));
                        // device_getBAR(&devbar, b, s, f, 4);
                        if (devinfo->vendor != 0xFFFF) {
                            printf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\n",
                                devinfo->vendor,
                                devinfo->device,
                                devinfo->command,
                                devinfo->status,
                                devinfo->revision,
                                devinfo->interface,
                                devinfo->subclass,
                                devinfo->class);
                                // ((device_read(b, s, f, 3) >> 16) & 0xFF) >> 7
                                // (size_t)devbar.addr
                        }
                    }
                }
            }
            port_outb(0x3F8, 'H');
            port_outb(0x3F8, 'e');
            port_outb(0x3F8, 'l');
            port_outb(0x3F8, 'l');
            port_outb(0x3F8, 'o');
            return 0;
            extern void usb_test(); usb_test(); return 0;
            extern void usb_mouse();
            usb_mouse(); return 0;
            device_t usbdev; device_get(&usbdev, 0, 1, 2);
            // usb_init(&usbdev);
            printf("USBCMD: 0x%x\n", port_inw(usbdev.port + 0x0));
            printf("USBSTS: 0x%x\n", port_inw(usbdev.port + 0x2));
            printf("USBINTR: 0x%x\n", port_inw(usbdev.port + 0x4));
            printf("FRNUM: 0x%x\n", port_inw(usbdev.port + 0x6));
            printf("FRBASE: 0x%x\n", port_inl(usbdev.port + 0x8));
            printf("SOFMOD: 0x%x\n", port_inb(usbdev.port + 0xC));
            printf("PORTSC1: 0x%x\n", port_inw(usbdev.port + 0x10));
            printf("PORTSC2: 0x%x\n", port_inw(usbdev.port + 0x12));
            // for (int i = 0; i < 1024; ++i) {
            //     uint32_t* ptr = (void*)port_inl(usbdev.port + 0x8);
            //     printf("%d: 0x%x\n", i, ptr[i]); // yield();
            // }
        } /* --> */ else {
            printf("Command %s not found\n", arg->v[0]); return EXIT_COMMAND_NOT_FOUND;
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
        if (my_data == NULL) { PANIC("Memory test error: Allocated memory is null"); }
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
    if (false) {
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
            printf("%s ", kernel_WorkingPath); char* cmd = prompt(getuid() ? "$ " : "# "); putchar('\n');
            if (split(cmd, ' ')->v[0] && compare(split(cmd, ' ')->v[0], "exit") == 0)
                { printf("Process completed\n"); break; }
            if (kernel_commandHandler(kernel_WorkingPath, cmd) != EXIT_SUCCESS) { /* Handle error */ }
            --kernel_CommandRecursionLevel;
        }
    }

    if (spawn("init", init) == -1) { printf("Unable to spawn process init\n"); } yield();

    PANIC("No processes to execute");    // Switch to idle if no process
}

// Initialize function of kernel
void kernel_init(multiboot_info_t* boot_info, multiboot_uint32_t boot_magic) {
    display_init();     // Initialize display driver

    // * Check multiboot magic number
    if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC) { PANIC("Invalid multiboot magic number"); }

    // * Check system memory map
    if (!(boot_info->flags >> 6 & 0x01)) { PANIC("Invalid memory map given by bootloader"); }

    // * Detect hardware and identify
    // Check the kernel base address
    if (KERNEL_BASE != (size_t)&kernel_start) { PANIC("Invalid kernel base address"); }
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
    // Get information about CPU
    {   uint32_t eax, ebx, ecx, edx;
        // Vendor string (EAX=0)
        kernel_cpuid(0, 0, &eax, &ebx, &ecx, &edx);
        ncopy(kernel_CPUInfo.vendor + 0, &ebx, 4);
        ncopy(kernel_CPUInfo.vendor + 4, &edx, 4);
        ncopy(kernel_CPUInfo.vendor + 8, &ecx, 4);
        kernel_CPUInfo.vendor[12] = '\0';
        // Processor brand string (EAX=0x80000002,3,4)
        char brand[49] = {0};
        for (int i = 0; i < 3; i++) {
            kernel_cpuid(0x80000002 + i, 0, &eax, &ebx, &ecx, &edx);
            ncopy(brand + i * 16 + 0, &eax, 4);
            ncopy(brand + i * 16 + 4, &ebx, 4);
            ncopy(brand + i * 16 + 8, &ecx, 4);
            ncopy(brand + i * 16 + 12, &edx, 4);
        }
        ncopy(kernel_CPUInfo.brand, brand, 48);
        kernel_CPUInfo.brand[48] = '\0';
        // Basic features (EAX=1)
        kernel_cpuid(1, 0, &eax, &ebx, &ecx, &edx);
        // Flags
        kernel_CPUInfo.has_sse = (edx >> 25) & 1;   // SSE bit in EDX
        kernel_CPUInfo.has_avx = (ecx >> 28) & 1;   // AVX bit in ECX
        kernel_CPUInfo.has_vtx = (ecx >> 5) & 1;    // VMX bit in ECX (Intel VT-x)
        kernel_CPUInfo.has_aes = (ecx >> 25) & 1;   // AES bit in ECX
        // Number of logical processors (threads) (EBX bits 23:16)
        kernel_CPUInfo.threads = (ebx >> 16) & 0xff;
        // Number of cores (EAX=4, ECX=0)
        kernel_cpuid(4, 0, &eax, &ebx, &ecx, &edx);
        kernel_CPUInfo.cores = ((eax >> 26) & 0x3f) + 1;    // Number of cores - 1 + 1 = cores
        if (kernel_CPUInfo.cores == 0) { kernel_CPUInfo.cores = 1; }    // Fallback
        // If threads is zero, use core count
        if (kernel_CPUInfo.threads == 0) { kernel_CPUInfo.threads = kernel_CPUInfo.cores; }
        // CPU Architecture (EAX=0x80000001)
        kernel_cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx);
        // ECX bit 29 - LM (Long Mode)
        kernel_CPUInfo.has_x64 = (edx & (1 << 29)) != 0;
        // Frequency
        putcursor(display_getcursor()); puts("Calculating...");
        extern uint64_t utils_rdtsc();
        uint64_t tsc1, tsc2, time =
            (date().day * 24 * 60 * 60) +
            (date().hour * 60 * 60) +
            (date().min * 60) +
            date().sec + 1;
        while (true) {
            if ((date().day * 24 * 60 * 60) +
                (date().hour * 60 * 60) +
                (date().min * 60) +
                date().sec >= time) { break; }
        } tsc1 = utils_rdtsc();
        time =
            (date().day * 24 * 60 * 60) +
            (date().hour * 60 * 60) +
            (date().min * 60) +
            date().sec + 1;
        while (true) {
            if ((date().day * 24 * 60 * 60) +
                (date().hour * 60 * 60) +
                (date().min * 60) +
                date().sec >= time) { break; }
        } tsc2 = utils_rdtsc();
        kernel_CPUInfo.frequency = tsc2 - tsc1;
    } putchar('\n');

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
    printf("CPU Information:\n");
    printf("\tVendor: %s\n", kernel_CPUInfo.vendor);
    printf("\tBrand: %s\n", kernel_CPUInfo.brand);
    printf("\tFrequency: %dMHz\n", kernel_CPUInfo.frequency/1024/1024);
    printf("\tCores\tThreads\tSSE\tAVX\tVT-x\tAES\tx64\n");
    printf("\t%d\t%d\t%s\t%s\t%s\t%s\t%s\n",
        kernel_CPUInfo.cores,
        kernel_CPUInfo.threads,
        kernel_CPUInfo.has_sse ? "Yes" : "No",
        kernel_CPUInfo.has_avx ? "Yes" : "No",
        kernel_CPUInfo.has_vtx ? "Yes" : "No",
        kernel_CPUInfo.has_aes ? "Yes" : "No",
        kernel_CPUInfo.has_x64 ? "Yes" : "No");
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
    } if (!fieldFound) { PANIC("No available memory fields"); }  // Halt the system if no memory field for use
    puts("---- Scanning Ended ----\n");
    putchar('\n');

    // * Initialize kernel
    puts("---- Initializing Kernel ----\n");
    puts("Initializing Protected Mode..."); protect_init(); puts(" Success.\n");
    puts("Initializing Interrupt Manager..."); interrupts_init(); puts(" Success.\n");
    puts("Initializing Memory Manager..."); memory_init(fieldSize - kernel_size()); puts(" Success.\n");
    puts("Initializing Multitasking Manager..."); multitask_init(); puts(" Success.\n");
    puts("Initializing RAM file system..."); ramfs_init(); puts(" Success.\n");
    puts("Initializing System Call Manager..."); syscall_init(); puts(" Success.\n");
    puts("Initializing USB Controller...");
        device_t usbdev; device_get(&usbdev, 0, 1, 2); usb_init(&usbdev);
        puts(" Success.\n");
    puts("---- Initializing Ended ----\n");
    putchar('\n');

    // * Set hostname
    fill(kernel_Hostname, 0, KERNEL_HOSTNAMELEN + 1); copy(kernel_Hostname, "localhost");
    
    // * Initialize device files
    {
        if (ramfs_createDir("/dev/") != RAMFS_STATUS_SUCCESS) { PANIC("Unable to create directory /dev/"); }
        void* zero = malloc(MEMORY_BLOCKSIZE); if (zero == NULL) { PANIC("Unable to allocate memory"); }
        fill(zero, 0, MEMORY_BLOCKSIZE);
        if (ramfs_writeFile("/dev/keyboard", MEMORY_BLOCKSIZE, zero) != RAMFS_STATUS_SUCCESS)
            { PANIC("Unable to create device /dev/keyboard"); }
        free(zero); ramfs_Entry_t* dev_keyboard = ramfs_stat("/dev/keyboard");
        if (dev_keyboard == NULL) { PANIC("Unable to get device /dev/keyboard"); }
        dev_keyboard->ftype = RAMFS_TYPE_CHARDEV; dev_keyboard->devperm = O_RDONLY;
    }

    // * Initialize system directories
    {
        if (ramfs_createDir("/root/") != RAMFS_STATUS_SUCCESS) { PANIC("Unable to create directory /root/"); }
        if (ramfs_createDir("/home/") != RAMFS_STATUS_SUCCESS) { PANIC("Unable to create directory /home/"); }
    }

    // Print kernel boot success message and build version
    printf("Deputy Kernel Build %d, booted successfully.\n", KERNEL_BUILD);

    // printf("0x%x\n", pciCheckVendor(0, 0));
    // printf("0x%x\n", pciCheckVendor(0, 1));
    // printf("0x%x\n", pciCheckVendor(0, 2));
    // printf("0x%x\n", pciCheckVendor(0, 3));
    // printf("0x%x\n", device_read(0, 1, 2, 0));
    // printf("0x%x\n", device_read(0, 1, 2, 1));
    // printf("0x%x\n", device_read(0, 1, 2, 2));

    // uint32_t in1 = 0, in2 = 0;
    // sleep(1);
    // asm volatile ("rdtsc" : "=a"(in1));
    // sleep(1);
    // asm volatile ("rdtsc" : "=a"(in2));
    // printf("%dHz\n", (in2 - in1));
    // while(1);

    // #include "assets/snail.h"
    // display_BMPInfo_t* image_info = (void*)snail_bmp + 14;
    // printf("W: %d, H: %d, B: %d\n", image_info->width, image_info->height, image_info->bitCount);
    // display_renderBMP(snail_bmp,
    //     (DISPLAY_GUIWIDTH / 2) - (image_info->width / 2),
    //     (DISPLAY_GUIHEIGHT / 2) - (image_info->height / 2));

    // write_registers();
    // unsigned char color = 13;

    // extern char font8x8_basic[128][8];
    // init_vga_640x480x16();
    // clear_screen();
    // int ptr = 0; char chr = 'M';
    // int x = (ptr % DISPLAY_CLIWIDTH) * 8;
    // int y = (ptr / DISPLAY_CLIWIDTH) * 10;
    // if (chr < 0 || chr > 127) return;
    // for (int row = 0; row < DISPLAY_FONTHEIGHT; row++) {
    //     char row_bits = font8x8_basic[(int)chr][row];
    //     for (int col = 0; col < DISPLAY_FONTWIDTH; col++) {
    //         int bit = (row_bits >> col) & 1;
    //         uint32_t color = bit ? 0x07 : 0x00;
    //         display_putpixel(x + col, y + row, color);
    //     }
    // }
    // clear_screen();
    // for (int y = 0; y < 400; ++y) {
    //     for (int x = 0; x < 640; ++x) {
    //         putpixel(x, y, 0x0F);
    //     }
    // }

    // for (int p = 0; p < 4; p++) {
    //     set_plane(p);
    //     unsigned char bit = (color >> p) & 1;
    //     // belleğe yaz: bit = 1 → set bit, 0 → clear bit
    // }
    // for (int y = 0; y < 480; ++y) {
    //     for (int x = 0; x < 640; ++x) {
    //         putpixel(x, y, 0x0F); // beyaz (1111)
    //     }
    // }
    // for (int y = 0; y < 480; ++y) {
    //     for (int x = 0; x < 640; ++x) {
    //         putpixel2(x, y, 0x7);
    //     }
    // }
    // for (int i = 0; i < 640; ++i) {
    //     putpixel2(i, 240, 0x4);
    // }
    // while(1);

    // char* msg = "Hello, world!\n"; for (int i = 0; i < length(msg); ++i) { display_putchar(msg[i], i); }

    // char* msg = "Merhaba, dunya! Nasilsiniz? Iyimisiniz? Nasil gidiyor?";
    // for (int i = 0; i < length(msg); ++i) { display_putchar(msg[i], i); }
    // printf("Merhaba, d�nya! Nas�ls�n�z? Bir s�redir g�r��m�yoruz. - M. �erif �.\n");
    
    // spawn("test", test);
    // spawn("test1", test1);
    // spawn("test2", test2);
    // spawn("test3", test3);
    // yield();
    // asm volatile ("int %0" : : "i"(SYS_YIELD));
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
    // ramfs_writeFile("/welcome.txt", length("Welcome!"), "Welcome!");
    // int welc = open("/welcome.txt", O_RDWR | O_CREAT | O_EXCL); if (welc == -1) { kernel_panic("Error: Open file"); }
    // char* welcedit = "Hello! How are you?";
    // printf("%d bytes written\n", write(welc, welcedit, length(welcedit)));
    // char* welcmsg = (char*)malloc(4096); if (welcmsg == NULL) { kernel_panic("Error: Allocate memory"); }
    // fill(welcmsg, 0, 4096); printf("%d bytes readed: %s\n", read(welc, welcmsg, 4096), welcmsg); while(true);

    // spawn("testshell", testshell);
    // spawn("keyboard_process", keyboard_process);
    // yield();

    kernel_main();  // Switch to kernel main
}