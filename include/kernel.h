#pragma once

#include "multiboot.h"
#include "types.h"
#include "common.h"
#include "platform/i386/port.h"
#include "platform/i386/interrupts.h"
#include "sysapi/syscall.h"

// Kernel entry memory address
#define KERNEL_ENTRY 0x100000

// Macro for handling Kernel Panic Situations
// Prints an error message with file and line information, then halts the system
#define kernel_panic(format, ...) \
    do { \
        /* Print the panic message with the file name and line number */ \
        printf("Kernel panic: %s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__); \
        /* Enter an infinite loop that disables interrupts and halts the CPU */ \
        while (1) { asm volatile ("cli\n hlt"); }; \
    } while (0)

// Macro for rebooting system
// Prints a reboot message and reboots the system
#define kernel_reboot() \
    do { \
        /* Disable interrupts */ \
        asm volatile ("cli"); \
        /* Print reboot message for user and wait a bit */ \
        puts("Requested system reboot..."); sleep(1); \
        /* Send reset request to PS/2 keyboard command port */ \
        port_outb(0x64, 0xFE); \
        /* Enter an infinite loop if reboot fail */ \
        while (1) { asm volatile ("hlt"); } \
    } while (0)

// Function prototype for get kernel size
size_t kernel_getSize();

// Function prototype for handle command (Kernel Level)
int kernel_commandHandler(char* path, char* str);

// Enumeration for boot device numbers
enum KERNEL_BOOTDEVICETABLE {
    BOOTDEVICE_INVALID  = 0,    // Invalid Device (Unknown Device)
    BOOTDEVICE_HARDDISK = 1,    // Harddisk Device
    BOOTDEVICE_FLOPPY   = 2,    // Floppy Device
    BOOTDEVICE_CDROM    = 3,    // CDROM Device
    BOOTDEVICE_USB      = 4,    // USB Device
    BOOTDEVICE_NET      = 5,    // Network Device
    BOOTDEVICE_SCSI     = 6     // Small Computer System Interface
};

// Define boot device type
typedef enum KERNEL_BOOTDEVICETABLE kernel_bootdevice_t;

// Function prototypes for get memory size and boot device
size_t kernel_getMemorySize();      // Gives Memory Size
uint32_t kernel_getBootDevice();    // Gives Boot Device
char* kernel_getBootDeviceStr();    // Gives Boot Device (as string)

// Function prototype for initialize hardware detector
void kernel_initHWDetector(multiboot_info_t* info);     // Initialize Hardware Detector

// * Memory management

// Reserved space size
#define MEMORY_SIZE (8 * 1024 * 1024)

// Memory allocable block sizes and count
#define MEMORY_BLOCKSIZE (4 * 1024)                         // Block size
#define MEMORY_BLOCKCOUNT (MEMORY_SIZE / MEMORY_BLOCKSIZE)  // Block count

// Memory report message max length
#define MEMORY_REPORTMESSAGELENGTH 256

// Allocable memory block structure
struct memory_Block {
    size_t number;      // Number of block
    size_t count;       // Connected block count
    bool allocated;     // Allocate state
    void* entry;        // Block memory address
};

// Function prototypes
void memory_init();             // Initializes the memory manager
void* malloc(size_t size);      // Allocates memory
void free(void* allocated);     // Frees memory
char* memory_report();          // Writes a memory report

// * Multitasking

#define MULTITASK_MAX_PROCESS_LIMIT 32

#define MULTITASK_TIMER_CTRL 0x43
#define MULTITASK_TIMER_COUNTER0 0x40
#define MULTITASK_TIMER_PIT_FREQUENCY 1193182
#define MULTITASK_TIMER_FREQUENCY 100

struct multitask_Process {
    bool active;
    size_t inst;
    size_t size;
    void* base;
};

int multitask_startProcess(void* program_buffer, size_t memory_size);
int multitask_endProcess(uint32_t process_id);

int multitask_init();