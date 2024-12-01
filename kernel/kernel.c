#include "multiboot.h"
#include "kernel.h"

#include "types.h"
#include "common.h"

#include "gdt.h"
#include "port.h"
#include "interrupts.h"

#include "display.h"
#include "keyboard.h"

#include "ramfs.h"

// Kernel code start and end (Set in linker.ld)
extern char _start, _end;

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

// Kernel Default Command Handler
int kernel_commandHandler(char* str) {
    if (strlen(str) <= 0) { putchar('\n'); return 0; }
    char* cmd = str;
    char** argv = split(str);
    int argc = getStrTokenCount();
    if (strlen(argv[0]) <= 0) { putchar('\n'); return 0; }
    if (argc > 0) {
        if (argv[0] && strcmp(argv[0], "echo") == 0) {
            for (int i = 1; i < argc; ++i) {
                printf(argv[i]);
                putchar(' ');
            }
            putchar('\n'); return 0;
        } else if (argv[0] && strcmp(argv[0], "clear") == 0) {
            display_clear();
            putcursor(0);
            return 0;
        } else if (argv[0] && strcmp(argv[0], "ls") == 0) {
            struct ramfs_Directory* dir = ramfs_getDirectory();
            for (int i = 0; i < dir->file_count; ++i) { printf("%s\n", dir->file[i].name); }
        } else if (argv[0] && strcmp(argv[0], "cat") == 0) {
            char* data;
            for (int i = 1; i < argc; ++i) {
                data = ramfs_readFile(argv[i]);
                if (data == null) {
                    printf("File %s not found\n", argv[i]); return -1;
                } else {
                    printf("%s\n", data);
                }
            }
        } else if (argv[0] && strcmp(argv[0], "touch") == 0) {
            for (int i = 1; i < argc; ++i) {
                if (ramfs_writeFile(argv[i], "") == -1) { printf("Unable to write %s\n", argv[i]); return -1; }
            }
        } else if (argv[0] && strcmp(argv[0], "write") == 0) {
            char data[ramfs_MAX_FILE_SIZE];
            memset(data, 0, ramfs_MAX_FILE_SIZE);
            int ptr = 0;
            for (int i = 2; i < argc; ++i) {
                for (int j = 0; j < strlen(argv[i]); ++j) {
                    data[ptr] = argv[i][j]; ptr++;
                }
                data[ptr] = ' '; ptr++;
            }
            if (ramfs_writeFile(argv[1], data) == -1) { printf("Unable to write file\n"); return -1; }
        } else if (argv[0] && strcmp(argv[0], "rm") == 0) {
            for (int i = 1; i < argc; ++i) {
                if (ramfs_removeFile(argv[i]) == -1) { printf("File %s not found\n", argv[i]); return -1; }
            }
        } else if (argv[0] && strcmp(argv[0], "devtools") == 0) {
            if (argv[1] && strcmp(argv[1], "panic") == 0) {
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

// Kernel Main
void kernel_main(void) {
    puts("Welcome!\n");                 // Print Welcome Message for User
    putchar('\n');

    puts("---- Memory Test Started ----\n");
    char* my_data = (char*)memory_allocate();
    printf("Allocated a memory block.\n");
    // strcpy(my_data, "Hello, World!");
    snprintf(my_data, memory_BLOCKSIZE, "Hello, %s!", "World");
    printf("Writed data to allocated memory block.\n");
    printf("My data: %s\n", my_data);
    printf("Memory usage: %s\n", memory_reportMessage());
    memory_free(my_data);
    printf("Allocated memory freed.\n");
    printf("Memory usage: %s\n", memory_reportMessage());
    puts("---- Memory Test Ended ----\n");
    putchar('\n');

    puts("---- File System Test Started ----\n");
    struct ramfs_Directory* dir = ramfs_getDirectory();
    printf("File list:\n"); for (int i = 0; i < dir->file_count; ++i) { printf("%s\n", dir->file[i].name); }
    ramfs_writeFile("myfile.txt", "Merhaba, Dunya!");
    printf("'myfile.txt' created.\n");
    printf("File list:\n"); for (int i = 0; i < dir->file_count; ++i) { printf("%s\n", dir->file[i].name); }
    printf("'myfile.txt' content: %s\n", ramfs_readFile("myfile.txt"));
    ramfs_removeFile("myfile.txt");
    printf("'myfile.txt' removed.\n");
    printf("File list:\n"); for (int i = 0; i < dir->file_count; ++i) { printf("%s\n", dir->file[i].name); }
    puts("---- File System Test Ended ----\n");
    putchar('\n');

    while(1) {
        char* prompt = scanf("# ");
        putchar('\n');
        if (prompt && strcmp(prompt, "exit") == 0) { break; }
        if (kernel_commandHandler(prompt) == -1) { /* Handle error */ }
    }

    kernel_panic("Switched to idle");   // Switch to idle if no process
}

// Kernel Initialize
void kernel_init(multiboot_info_t* boot_info, uint32_t boot_magic) {
    display_enablecursor(14, 15);       // Enable Display Cursor
    display_clear();                    // Clear Display
    // Set Cursor to Bottom-Left Corner
    display_putcursor(0); putcursor(display_CLIWIDTH * (display_CLIHEIGHT - 1));

    if (boot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {             // Check Magic Number
        kernel_panic("Invalid multiboot magic number");
    }

    if (!(boot_info->flags >> 6 & 0x01)) {                      // Check Memory map
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

    kernel_size = (size_t)&_end - (size_t)&_start;
    printf("Kernel Size: %d byte\n", kernel_size);
    printf("Boot device: %d (%s)\n", kernel_getBootDevice(), kernel_getBootDeviceStr());
    putchar('\n');

    // Initialize Kernel
    puts("---- Initializing Kernel ----\n");
    puts("Initializing Global Descriptor Table...\n"); gdt_init();
    puts("Initializing Interrupt Manager...\n"); interrupts_init();
    puts("Initializing Hardware Detector...\n"); kernel_initHWDetector(boot_info);
    puts("Initializing Memory Manager...\n"); memory_init();
    puts("Initializing RAM File System...\n"); ramfs_init();
    puts("---- Initializing Ended ----\n");
    putchar('\n');
    // while(1);

    puts("Sheriff Kernel Build 23, booted successfully.\n");    // Print Kernel Boot Success Message and Build Version
    kernel_main();                                              // Switch to Kernel Main
}

// Function for get kernel size
size_t kernel_getSize() { return kernel_size; }


// void test_gdt() {
//     // Testing GDT by setting DS to invalid data segment
//     asm volatile(
//         "movw $0x12345, %ax;\n"
//         "movw %ax, %ds;"
//     );  // Invalid data segment
//     char *str = "Test";
//     str[0] = 'A';  // An error is expected here (General Protection Fault aka GPF)
// }
