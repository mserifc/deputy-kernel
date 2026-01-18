#include "kernel.h"

#include "platform/i386/port.h"
#include "platform/i386/gdt.h"
#include "platform/i386/interrupts.h"

#include "drivers/display.h"
#include "drivers/keyboard.h"
#include "drivers/disk.h"

#include "filesystem/ramfs.h"

#include "sysapi/syscall.h"

#include "bmp_24.h"
#include "snail.h"

// Kernel code start and end (Set in linker.ld)
extern char kernel_start, kernel_end;

// Kernel Size
size_t kernel_size;

// Memory Lower (Below 1MB) and Upper (Above 1MB) Sizes
size_t kernel_MemoryLowerSize;
size_t kernel_MemoryUpperSize;

// Boot Device Number
uint32_t kernel_BootDevice;

// Boot Devices (as string)
char* kernel_BootDeviceStr[] = {
    "Invalid",
    "Harddisk",
    "Floppy",
    "CDROM",
    "USB",
    "Network",
    "SCSI"
};

// Working shell path
char path[RAMFS_MAX_NAME_LENGTH + 1];

// Check for file system is saved or not
bool kernel_SaveFSSession = false;

// Function for get kernel size
size_t kernel_getSize() { return kernel_size; }

// Function for get memory size
size_t kernel_getMemorySize() { return kernel_MemoryLowerSize + kernel_MemoryUpperSize; }

// Function for get boot device number
uint32_t kernel_getBootDevice() { return kernel_BootDevice; }

// Function for get boot device as string
char* kernel_getBootDeviceStr() { return kernel_BootDeviceStr[kernel_getBootDevice()]; }

// Function for initialize hardware detector
void kernel_initHWDetector(multiboot_info_t* info) {
    kernel_MemoryLowerSize = info->mem_lower;   // Get lower (Below 1MB) memory size
    kernel_MemoryUpperSize = info->mem_upper;   // Get upper (Above 1MB) memory size
    if (info->flags & (1 << 4)) {               // Check boot device flag
        kernel_BootDevice = info->boot_device;  // Get boot device if boot device flag set
    } else {
        kernel_BootDevice = 0;                  // Else set boot device invalid
    }
}

// Function for list file system directory structure
void test_listastree() {
    struct ramfs_Root* root = ramfs_getRoot();
    printf("Listing directory structure:\n");
    printf("    root:\n");
    for (int i = 0; i < root->dir_count; ++i) {
        printf("        %s:\n", root->dir[i].name);
        for (int j = 0; j < root->dir[i].file_count; ++j) {
            printf("            %s\n", root->dir[i].file[j].name);
        }
    }
};

// Example function for debug tests
int test_exmfunc(uint32_t value) {
    printf("Example function successfully called. Listing information:\n");
    printf("    First argument value (as decimal): %d\n", value);
    return 0;
}

void test_exmtask1() {
    while (true) {
        printf("Example task 1 running...\n");
        sleep(1);
        yield();
        printf("Returned to example task 1\n");
    }
}

void test_exmtask2() {
    while (true) {
        printf("Example task 2 working...\n");
        sleep(1);
        yield();
        printf("Returned to example task 2\n");
    }
}

void test_exmtask3() {
    while (true) {
        printf("Example task 3 in focus...\n");
        sleep(1);
        yield();
        printf("Returned to example task 3\n");
    }
}

// Kernel command handler
int kernel_commandHandler(char* path, char* str) {
    if (strlen(str) <= 0) { putchar('\n'); return 0; }
    char* cmd = str;
    char** argv = split(str, ' ');
    int argc = getStrTokenCount();
    if (strlen(argv[0]) <= 0) { putchar('\n'); return 0; }
    if (argc > 0) {
        if (argv[0] && strcmp(argv[0], "echo") == 0) {
            for (int i = 1; i < argc; ++i) {
                printf(argv[i]);
                putchar(' ');
            }
            putchar('\n'); return 0;
        } else if (argv[0] && strcmp(argv[0], "date") == 0) {
            printf("%d:%d:%d %d/%d/%d\n",
                date().hour, date().min, date().sec, date().day, date().mon, date().year);
        } else if (argv[0] && strcmp(argv[0], "sleep") == 0) {
            sleep(atoi(argv[1]));
        } else if (argv[0] && strcmp(argv[0], "cd") == 0) {
            if (argv[0] && strcmp(argv[0], "cd") == 0) {
                if (argc > 1) {
                    if (argv[1] && strcmp(argv[1], ".") == 0) { return 0; }
                    if (
                        argv[1] &&
                        strcmp(argv[1], "..") == 0 ||
                        strcmp(argv[1], "/") == 0
                    ) { strcpy(path, "/"); return 0; }
                    if (argv[1][0] == '/') {
                        if (ramfs_getDirIndex(&argv[1][1]) != -1) {
                            snprintf(path, RAMFS_MAX_NAME_LENGTH + 1, "/%s", &argv[1][1]);
                        } else { printf("Directory %s not found\n", argv[1]); return -1; }
                    } else {
                        if (ramfs_getDirIndex(argv[1]) != -1) {
                            snprintf(path, RAMFS_MAX_NAME_LENGTH + 1, "/%s", argv[1]);
                        } else { printf("Directory %s not found\n", argv[1]); return -1; }
                    }
                } else { strcpy(path, "/"); }
                return -1;
            }
        } else if (argv[0] && strcmp(argv[0], "pwd") == 0) {
            printf("%s\n", path);
        } else if (argv[0] && strcmp(argv[0], "lstree") == 0) {
            struct ramfs_Root* root = ramfs_getRoot();
            printf("/\n");
            for (int i = 0; i < root->dir_count; ++i) {
                printf("    %s/\n", root->dir[i].name);
                for (int j = 0; j < root->dir[i].file_count; ++j) {
                    printf("        %s\n", root->dir[i].file[j].name);
                }
            }
        } else if (argv[0] && strcmp(argv[0], "ls") == 0) {
            if (argc == 1) {
                if (path && strcmp(path, "/") == 0) {
                    for (int i = 0; i < ramfs_getRoot()->dir_count; ++i) {
                        printf("%s\n", ramfs_getRoot()->dir[i].name);
                    }
                } else {
                    struct ramfs_Directory* dir = ramfs_getDir(&path[1]);
                    if (dir != NULL) {
                        for (int i = 0; i < dir->file_count; ++i) {
                            printf("%s\n", dir->file[i].name);
                        }
                        return 0;
                    } else { printf("Working directory not found\n"); return -1; }
                }
            } else if (argc == 2) {
                if (argv[1] && strcmp(argv[1], "/") == 0) {
                    for (int i = 0; i < ramfs_getRoot()->dir_count; ++i) {
                        printf("%s\n", ramfs_getRoot()->dir[i].name);
                    }
                    return 0;
                }
                for (int i = 0; i < strlen(argv[1]); ++i) {
                    if (argv[1][i] == '/') { printf("File system does not support multi-level directories\n"); return -1; }
                }
                struct ramfs_Directory* dir = ramfs_getDir(argv[1]);
                if (dir != NULL) {
                    for (int i = 0; i < dir->file_count; ++i) {
                        printf("%s\n", dir->file[i].name);
                    }
                    return 0;
                } else { printf("Directory not found\n"); return -1; }
            } else { printf("Too much arguments\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "cat") == 0) {
            if (argc > 1) {
                if (path && strcmp(path, "/") == 0) { printf("Operation not supported by the file system\n"); return -1; }
                struct ramfs_Directory* dir = ramfs_getDir(&path[1]);
                if (dir == NULL) { printf("Working directory not found\n"); return -1; }
                for (int i = 1; i < argc; ++i) {
                    if (ramfs_readFile(&path[1], argv[i]) != NULL) {
                        printf("%s", ramfs_readFile(&path[1], argv[i]));
                    } else { printf("File %s not found\n", argv[i]); return -1; }
                }
                putchar('\n'); return 0;
            } else { printf("Too few arguments\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "mkdir") == 0) {
            if (path && strcmp(path, "/") == 0) {
                for (int i = 0; i < strlen(argv[1]); ++i) {
                    if (argv[1][i] == '/') { printf("File system does not support multi-level directories\n"); return -1; }
                }
                if (ramfs_createDir(argv[1]) == -1) { printf("Unable to create directory %s\n", argv[1]); return -1; }
            } else { printf("File system does not support multi-level directories\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "rmdir") == 0) {
            if (path && strcmp(path, "/") == 0) {
                for (int i = 0; i < strlen(argv[1]); ++i) {
                    if (argv[1][i] == '/') { printf("Unable to remove directory\n"); return -1; }
                }
                if (ramfs_removeDir(argv[1]) == -1) {printf("Unable to remove direcotry: Directory not found\n");return -1;}
            } else { printf("Unable to remove directory: Directory not found\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "touch") == 0) {
            if (argc > 1) {
                if (path && strcmp(path, "/") != 0) {
                    for (int i = 0; i < strlen(argv[1]); ++i) {
                        if (argv[1][i] == '/') { printf("Operation not supported\n"); return -1; }
                    }
                    for (int i = 1; i < argc; ++i) {
                        if (ramfs_writeFile(&path[1], argv[i], "", 1) == -1) { printf("Unable to create file %s\n", argv[i]); }
                    }
                } else { printf("Operation not supported by the file system\n"); return -1; }
            } else { printf("Too few arguments\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "write") == 0) {
            if (path && strcmp(path, "/") == 0) {
                printf("Operation not supported by the file system\n"); return -1;
            } else if (argc <= 2) {
                printf("Too few arguments\n"); return -1;
            } else {
                for (int i = 0; i < strlen(argv[1]); ++i) {
                    if (argv[1][i] == '/') { printf("Operation not supported by the file system\n"); return -1; }
                }
                if (ramfs_getDir(&path[1]) == NULL) { printf("Working directory not found\n"); return -1; }
                char* buffer = (char*)malloc(MEMORY_BLOCKSIZE);
                int ptr = 0;
                for (int i = 2; i < argc; ++i) {
                    int len = strlen(argv[i]);
                    for (int j = 0; j < len; ++j) {
                        buffer[ptr] = argv[i][j];
                        ptr++;
                    }
                    buffer[ptr] = ' ';
                    ptr++;
                }
                buffer[ptr] = '\0';
                if (ramfs_writeFile(&path[1], argv[1], buffer, MEMORY_BLOCKSIZE) == -1) {
                    printf("Unable to write file %s\n", argv[1]);
                    return -1;
                };
            }
        } else if (argv[0] && strcmp(argv[0], "rm") == 0) {
            if (argc > 1) {
                if (path && strcmp(path, "/") != 0) {
                    for (int i = 0; i < strlen(argv[1]); ++i) {
                        if (argv[1][i] == '/') { printf("Operation not supported\n"); return -1; }
                    }
                    for (int i = 1; i < argc; ++i) {
                        if (ramfs_removeFile(&path[1], argv[i]) == -1) { printf("Unable to remove file %s\n", argv[i]); }
                    }
                } else { printf("Operation not supported by the file system\n"); return -1; }
            } else { printf("Too few arguments\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "clear") == 0) {
            display_clear();
            putcursor(0);
            return 0;
        } else if (argv[0] && strcmp(argv[0], "devtools") == 0) {
            if (argv[1] && strcmp(argv[1], "mem") == 0) {
                printf("Memory usage: %s\n", memory_report());
            } else if (argv[1] && strcmp(argv[1], "panic") == 0) {
                kernel_panic("Manually triggered");
            } else {
                printf("Unknown developer tool.\n");
                return -1;
            }
        } else {
            printf("Command %s not found\n", argv[0]);
            return -1;
        }
    } else { putchar('\n'); return 0; }
    return 0;
}

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
        // strcpy(my_data, "Hello, World!");
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

    // * Disk driver test
    if (false) {
        puts("---- Disk test started ----\n");
        if (disk_support()) {
            char* my_data = (char*)malloc(DISK_SECTOR_SIZE);
            printf("Allocated a memory buffer.\n");
            snprintf(my_data, DISK_SECTOR_SIZE, "Hello, %s!", "world");
            printf("Writed data to allocated memory buffer.\n");
            if (disk_writeSector(0, my_data) == -1) {
                kernel_panic("Disk test error: Disk writing sector failed");
            }
            printf("Writed allocated memory buffer to disk's first sector.\n");
            memset(my_data, 0, DISK_SECTOR_SIZE);
            snprintf(my_data, DISK_SECTOR_SIZE, "Hi, %s!", "Serif");
            printf("Writed data to allocated memory buffer.\n");
            if (disk_writeSector(1, my_data) == -1) {
                kernel_panic("Disk test error: Disk writing sector failed");
            }
            printf("Writed allocated memory buffer to disk's second sector.\n");
            memset(my_data, 0, DISK_SECTOR_SIZE);
            printf("Allocated memory buffer cleaned\n");
            if (disk_readSector(0, my_data) == -1) {
                kernel_panic("Disk test error: Disk reading sector failed");
            }
            printf("Readed data from disk's first sector: %s\n", my_data);
            if (disk_readSector(1, my_data) == -1) {
                kernel_panic("Disk test error: Disk reading sector failed");
            }
            printf("Readed data from disk's second sector: %s\n", my_data);
            free(my_data);
            printf("Allocated memory buffer freed.\n");
        } else { puts("Disk test failed: Device does not support ATA disk controller.\n"); }
        puts("---- Disk test ended ----\n");
        putchar('\n');
    }

    // * File system test
    if (false) {
        puts("---- File system test started ----\n");
        printf("Requested to create a directory (code: %d)\n", ramfs_createDir("docs"));
        test_listastree();
        // sleep(3);
        printf("Requested to create a directory (code: %d)\n", ramfs_createDir("docs"));
        test_listastree();
        // sleep(3);
        printf("Requested to write a file (code: %d)\n", ramfs_writeFile("docs", "readme.txt", "Hello, world!", MEMORY_BLOCKSIZE));
        test_listastree();
        printf("Printing content of /docs/readme.txt: %s\n", ramfs_readFile("docs", "readme.txt"));
        // sleep(3);
        printf("Requested to write a file (code: %d)\n", ramfs_writeFile("docs", "readme.txt", "How are you?", MEMORY_BLOCKSIZE));
        test_listastree();
        printf("Printing content of /docs/readme.txt: %s\n", ramfs_readFile("docs", "readme.txt"));
        // sleep(3);
        printf("Requested to remove a file (code: %d)\n", ramfs_removeFile("docs", "readme.txt"));
        test_listastree();
        printf("Finding removed file (code %d)\n", ramfs_getFileIndex("docs", "readme.txt"));
        // sleep(3);
        printf("Requested to remove a removed file (code: %d)\n", ramfs_removeFile("docs", "readme.txt"));
        test_listastree();
        // sleep(3);
        printf("Requested to remove a directory (code: %d)\n", ramfs_removeDir("docs"));
        test_listastree();
        printf("Finding removed directory (code %d)\n", ramfs_getDirIndex("docs"));
        // sleep(3);
        printf("Requested to remove a removed directory (code: %d)\n", ramfs_removeDir("docs"));
        test_listastree();
        // sleep(3);
        puts("---- File system test ended ----\n");
        putchar('\n');
    }

    // * System call manager test
    if (false) {
        puts("---- System call manager test started ----\n");
        printf("Adding new system call to first entry (number 1)...");
        syscall_addEntry(1, (size_t)test_exmfunc);
        printf(" Success.\n");
        printf("Calling added system call... (EAX: 1, EBX: 76)\n");
        asm volatile (
            "mov $1, %%eax\n\t"
            "mov $76, %%ebx\n\t"
            "int $0x80\n\t"
            :
            :
            : "%eax"
        );
        int result;
        asm volatile ("mov %%eax, %0" : "=r"(result) : : "%eax");
        printf("    Return value (EAX): %d\n", result);
        printf("Removing added system call (number 1)...");
        syscall_addEntry(1, 0x00);
        printf(" Success.\n");
        puts("---- System call manager test ended ----\n");
        putchar('\n');
    }

    if (0) {
        char mymsg[100];
        asm volatile (
            "mov $3, %%eax\n\t"
            "mov %0, %%ebx\n\t"
            "mov $10, %%ecx\n\t"
            "int $0x80\n\t"
            :
            : "r"(mymsg)
            : "%eax", "%ebx", "%ecx"
        );
        asm volatile (
            "mov $4, %%eax\n\t"
            "mov %0, %%ebx\n\t"
            "mov %1, %%ecx\n\t"
            "int $0x80\n\t"
            :
            : "r"(mymsg), "r"(strlen(mymsg))
            : "%eax", "%ebx", "%ecx"
        );
    }

    // * Colorful graphic display test
    if (false) {
        display_graphic_switch();
        uint8_t color = 0;
        while (true) {
            display_graphic_fillRect(0, 0, 320, 200, color);
            if (color >= 255) {
                color = 0;
            } else { color++; }
            sleep(1);
        }
    }

    // * Bitmap image viewer test
    if (true) {
        display_graphic_switch();
        display_graphic_bmpViewer(snail_bmp);
    }

    // Loading file system session from disk
    // puts("Loading file system session from disk...");
    // if (ramfs_load() == -1) {
    //     puts("\nAn error occured while loading the file system session from disk.\n");
    //     kernel_SaveFSSession = false;
    // } else { kernel_SaveFSSession = true; puts(" Success.\n"); }

    // * Built-in kernel shell
    if (true) {
        puts("Unable to run user shell, switching to built-in kernel shell.\n");
        ramfs_writeFile("system", "kernelversion.txt", "Sheriff Kernel Build 29", MEMORY_BLOCKSIZE);
        ramfs_writeFile("system", "bootlog.txt", "Sheriff Kernel Build 29, booted successfully.", MEMORY_BLOCKSIZE);
        ramfs_writeFile("system", "readme.txt", "Thanks for using my kernel ;)", MEMORY_BLOCKSIZE);
        strcpy(path, "/");
        char* header;
        while(1) {
            snprintf(header, RAMFS_MAX_NAME_LENGTH + 3, "%s # ", path);
            char* prompt = scanf(header);
            putchar('\n');
            if (prompt && strcmp(prompt, "exit") == 0) { printf("Process completed\n"); break; }
            if (kernel_commandHandler(path, prompt) == -1) { /* Handle error */ }
        }
    }

    // Saving file system session to disk
    // if (kernel_SaveFSSession) {
    //     puts("Saving file system session to disk...");
    //     if (ramfs_save() == -1) {
    //         puts("An error occured while loading the file system session to disk.\n");
    //     } else { puts(" Success.\n"); }
    // }

    ramfs_disable();

    kernel_panic("No processes to execute");    // Switch to idle if no process
}

void kernel_init(multiboot_info_t* boot_info, uint32_t boot_magic) {
    display_init();     // Initialize display driver
    display_clear();    // Clear display
    // Set cursor to bottom-left corner
    display_putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));  // Set on display
    putcursor(DISPLAY_CLIWIDTH * (DISPLAY_CLIHEIGHT - 1));          // Set on common libraries

    if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {                 // Check magic number
        kernel_panic("Invalid multiboot magic number");
    }

    if (!(boot_info->flags >> 6 & 0x01)) {                          // Check memory map
        kernel_panic("Invalid memory map given by bootloader");
    }

    // Scan for available memory block
    // Normally, no need that for this kernel but I added it anyway so you can add to your own project.
    if (0) {    // Do not start
        puts("---- Scanning Memory ----\n");
        puts("Scanning for a memory block to use...\n");
        for (int i = 0; i < boot_info->mmap_length; i += sizeof(multiboot_memory_map_t)) {
            multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (boot_info->mmap_addr + i);
            printf("Start: 0x%x, Length: 0x%x, Size: 0x%x, Type: %d\n",
                mmmt->addr, mmmt->len, mmmt->size, mmmt->type);
            if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
                if (mmmt->addr < 0x100000 && mmmt->size < 1*1024*1024) {
                    printf("Available memory block found but its too small.\n");
                } else {
                    printf("Available memory block found.\n");
                }
            } else {
                printf("Not available.\n");
            }
        }
        puts("---- Scanning Ended ----\n");
        putchar('\n');
    }

    // Print information about device and kernel
    kernel_size = (size_t)&kernel_end - (size_t)&kernel_start;  // Calculate kernel size
    printf("Kernel Size: %d byte\n", kernel_size);              // Print kernel size
    printf("Boot device: %d (%s)\n",                            // Print boot device
        kernel_getBootDevice(), kernel_getBootDeviceStr());
    printf("Date: %d:%d:%d %d/%d/%d\n",                         // Print date
        date().hour, date().min, date().sec, date().day, date().mon, date().year);
    putchar('\n');

    // Initialize Kernel
    puts("---- Initializing Kernel ----\n");
    puts("Initializing Protected Mode..."); gdt_init(); puts(" Success.\n");
    puts("Initializing Interrupt Manager..."); interrupts_init(); puts(" Success.\n");
    puts("Detecting Hardware..."); kernel_initHWDetector(boot_info); puts(" Success.\n");
    if (!disk_support()) { puts("Warning: Device does not support ATA disk controller, ignoring.\n"); }
    puts("Initializing Memory Manager..."); memory_init(); puts(" Success.\n");
    puts("Initializing RAM File System..."); ramfs_init(); puts(" Success.\n");
    puts("Initializing System Call Manager..."); syscall_init(); puts(" Success.\n");
    puts("Initializing Multitasking Manager..."); multitask_init(); puts(" Success.\n");
    puts("---- Initializing Ended ----\n");
    putchar('\n');

    puts("Sheriff Kernel Build 29, booted successfully.\n");    // Print kernel boot success message and build version
    // * Multitasking system test
    if (false) {
        printf("%s\n", memory_report());
        void* task1 = malloc(32*1024);
        printf("%d\n", (size_t)task1);
        void* task2 = malloc(32*1024);
        printf("%d\n", (size_t)task2);
        void* task3 = malloc(32*1024);
        printf("%d\n", (size_t)task3);
        if (task1 == NULL || task2 == NULL || task3 == NULL) { kernel_panic("Memory allocation failed"); }
        memcpy(task1, test_exmtask1, 32*1024);
        memcpy(task2, test_exmtask2, 32*1024);
        memcpy(task3, test_exmtask3, 32*1024);
        printf("PID: %d\n", spawn(task1, 32*1024));
        printf("PID: %d\n", spawn(task2, 32*1024));
        printf("PID: %d\n", spawn(task3, 32*1024));
        printf("%s\n", memory_report());
        yield();
        while(1);
    }
    kernel_main();                                              // Switch to kernel main
}