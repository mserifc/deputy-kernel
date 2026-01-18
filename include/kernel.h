#pragma once

#include "types.h"
#include "utils.h"
#include "debug.h"

// Kernel code start and end (Set in linker.ld)
extern char kernel_start, kernel_end;

// Constants

#define KERNEL_BASE                 0x100000    // Kernel base address
#define KERNEL_BUILD                30          // Kernel build version

#define KERNEL_BOOTDEVICE_INVALID   0x00        // Invalid boot device
#define KERNEL_BOOTDEVICE_HARDDISK  0x01        // Harddisk boot device
#define KERNEL_BOOTDEVICE_FLOPPY    0x02        // Floppy boot device
#define KERNEL_BOOTDEVICE_CDROM     0x03        // CDROM boot device
#define KERNEL_BOOTDEVICE_USB       0x04        // USB boot device
#define KERNEL_BOOTDEVICE_NETWORK   0x05        // Network boot device
#define KERNEL_BOOTDEVICE_SCSI      0x06        // SCSI (Small Computer System Interface) boot device

#define KERNEL_TEXTEDIT_BUFFSIZE    (4 * 1024)  // Buffer size of built-in text editor

// Enable/disable kernel graphic mode
#define KERNEL_GRAPHICMODE false

// Exit codes

#define EXIT_SUCCESS                0       // Success
#define EXIT_FAILURE                1       // General failure
#define EXIT_USAGE_ERROR            2       // Usage error
#define EXIT_UNKNOWN_ERROR          3       // Unknown error
#define EXIT_IO_ERROR               5       // I/O error
#define EXIT_INVAILD_CONFIG         99      // Invalid configuration
#define EXIT_PERMISSION_DENIED      126     // Permission denied
#define EXIT_COMMAND_NOT_FOUND      127     // Command not found
#define EXIT_INVALID_STATUS         128     // Invalid status
#define EXIT_PROCESS_INTERRUPTED    130     // Process interrupted
#define EXIT_PROCESS_KILLED         137     // Process killed
#define EXIT_MEMORY_VIOLATION       139     // Memory violation
#define EXIT_OUT_OF_RANGE           255     // Out of range error

// Macro functions

// Macro for handling kernel panic situations
// Prints an error message with file and line information, then halts the system
#define kernel_panic(format, ...) \
    do { \
        /* Print the panic message with the file name and line number */ \
        printf("Kernel panic: %s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__); \
        /* Enter an infinite loop that disables interrupts and halts the CPU */ \
        while (1) { asm volatile ("cli\n hlt"); }; \
    } while (0)

// Macro for reseting system
// Prints a system reset message and resets the system
#define kernel_reset() \
    do { \
        /* Disable interrupts */ \
        asm volatile ("cli"); \
        /* Print system reset message for user and wait a bit */ \
        puts("Requested system reset..."); sleep(1); \
        /* Send reset request to PS/2 keyboard command port */ \
        port_outb(0x64, 0xFE); \
        /* Enter an infinite loop if system reset fail */ \
        while (1) { asm volatile ("hlt"); } \
    } while (0)

// Public functions

size_t      kernel_size();      // Get kernel size
size_t      kernel_memsize();   // Get physical memory size
uint32_t    kernel_bootdev();   // Get boot device

int kernel_textEditor(char* path);                  // Built-in kernel text editor
int kernel_commandHandler(char* path, char* cmd);   // Built-in kernel command handler

// * Memory Management

// Minimum memory size
#define MEMORY_MINIMUMSIZE (4 * 1024 * 1024)

// Allocable memory block size
#define MEMORY_BLOCKSIZE (4 * 1024)

// Allocable memory block structure
typedef struct {
    size_t count;
    bool allocated;
    void* entry;
} memory_Block_t;

// Public functions

void*   malloc(size_t size);                        // Allocates memory
// void*   calloc(size_t size);                        // Allocates cleared memory
void*   realloc(void* allocated, size_t size);      // Reallocates memory
void    free(void* allocated);                      // Frees memory
void    memory_init(uint64_t size);                 // Initializes memory manager
char*   memory_report();                            // Writes a memory report
memory_Block_t* memory_blkInfo(void* allocated);    // Returns block information

// * Process Management

// Public functions

void    process_init();                             // Initializes process manager
int     spawn(char* name, void* program);           // Spawns a process
int     kill(int pid);                              // Kills a process
void    yield();                                    // Switch next process
void    exit();                                     // End process

// * System Call Management

// List of system calls
#define SYS_EXIT        0x01                        // End current process
#define SYS_SPAWN       0x02                        // Spawn a process
#define SYS_READ        0x03
#define SYS_WRITE       0x04
// #define SYS_YIELD 0x9E

#define STDIN           0                           // Standard input
#define STDOUT          1                           // Standard output

void syscall_init();                                // Initializes system call manager
size_t read(int fd, void* buf, size_t count);
size_t write(int fd, void* buf, size_t count);