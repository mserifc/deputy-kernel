#include "multiboot.h"
#include "kernel.h"

#include "types.h"
#include "gdt.h"
#include "port.h"
#include "interrupts.h"
#include "detect.h"
#include "display.h"
#include "keyboard.h"
#include "common.h"

// Kernel code start and end (Set in linker.ld)
extern char _start, _end;

// Kernel Size
size_t kernel_size;

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

    kernel_panic("Switched to idle");   // Switch to idle if no process
}

// Kernel Initialize
void kernel_init(multiboot_info_t* boot_info, uint32_t boot_magic) {
    display_enablecursor(14, 15);       // Enable Display Cursor
    display_clear();                    // Clear Display
    display_putcursor(0);               // Set Cursor to Top-Left Corner

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
    printf("Boot device: %d (%s)\n", detect_getBootDevice(), detect_getBootDeviceStr());
    putchar('\n');

    // Initialize Kernel
    puts("---- Initializing Kernel ----\n");
    puts("Initializing Global Descriptor Table...\n"); gdt_init();
    puts("Initializing Interrupt Manager...\n"); interrupts_init();
    puts("Initializing Hardware Detector...\n"); detect_init(boot_info);
    puts("Initializing Memory Manager...\n"); memory_init();
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
