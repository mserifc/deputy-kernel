#pragma once

#include "multiboot.h"
#include "common.h"

// Macro for handling Kernel Panic Situations
// Prints an error message with file and line information, then halts the system
#define kernel_panic(format, ...) \
    do { \
        /* Print the panic message with the file name and line number */ \
        printf("Kernel panic: %s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__); \
        /* Enter an infinite loop that disables interrupts and halts the CPU */ \
        while(1) { asm volatile ("cli\n hlt"); }; \
    } while (0)

// Function prototype for get kernel size
size_t kernel_getSize();

// Function prototype for handle command (Kernel Level)
int kernel_commandHandler(char* str);

// Enumeration for boot device numbers
enum kernel_BOOTDEVICETABLE {
    BOOTDEVICE_INVALID  = 0,    // Invalid Device (Unknown Device)
    BOOTDEVICE_HARDDISK = 1,    // Harddisk Device
    BOOTDEVICE_FLOPPY   = 2,    // Floppy Device
    BOOTDEVICE_CDROM    = 3,    // CDROM Device
    BOOTDEVICE_USB      = 4,    // USB Device
    BOOTDEVICE_NET      = 5,    // Network Device
    BOOTDEVICE_SCSI     = 6     // Small Computer System Interface
};

typedef enum kernel_BOOTDEVICETABLE kernel_BootDeviceType;  // Define boot device type

// Function prototypes for get memory size and boot device
size_t kernel_getMemorySize();      // Gives Memory Size
uint32_t kernel_getBootDevice();    // Gives Boot Device
char* kernel_getBootDeviceStr();    // Gives Boot Device (as string)

// Function prototype for initialize hardware detector
void kernel_initHWDetector(multiboot_info_t* info);   // Initialize Hardware Detector

// Memory Management

// Reserved space entry address and size for Memory Manager
// #define memory_ENTRY 0x100000                               // Entry address (After 1MB)
#define memory_SIZE (1 * 1024 * 1024)                       // Size (1MB Size)

// Allocable memory block size and count
#define memory_BLOCKSIZE (16 * 1024)                        // Allocable block sizes (16KB Blocks)
#define memory_BLOCKCOUNT (memory_SIZE / memory_BLOCKSIZE)  // Allocable block count (Memory Size / Allocable Block Size)

#define memory_REPORTMSGLENGTH 100                          // Memory report message length

// Allocable Block Structure
struct memory_Block {
    size_t number;      // Block Number
    bool allocated;     // Block Status (Is Allocated or not)
    void* entry;        // Block Entry Address
};

// Memory report structure
struct memory_Report_t {
    size_t using_blocks;    // Allocated blocks
    size_t using_size;      // Allocated size
    size_t empty_blocks;    // Free blocks
    size_t empty_size;      // Free size
};

// Function for initialize memory
void memory_init();                     // Initializes the Memory Manager

// Function for allocate Memory Block
void* memory_allocate();                // Allocates a allocable memory block

// Function for free the specific allocated memory block
void memory_free(void* allocated);      // Frees the specific allocated memory block

// Functions for get memory report
struct memory_Report_t memory_report(); // Gives a memory report
char* memory_reportMessage();           // Writes a memory report
