#pragma once

#include "types.h"
#include "utils.h"
#include "debug.h"

// Kernel code start and end (Set in linker.ld)
extern char kernel_start, kernel_end;

// Constants

#define KERNEL_BASE                 0x100000    // Kernel base address
#define KERNEL_BUILD                31          // Kernel build version

#define KERNEL_NAME         "Deputy"
#define KERNEL_RELEASE      "build_" NUMBER(KERNEL_BUILD)
#define KERNEL_VERSION      "Deputy Kernel Build " NUMBER(KERNEL_BUILD) " (Jul 2025)"
#define KERNEL_PLATFORM     "deputy/i386"

#define KERNEL_BOOTDEVICE_INVALID   0x00        // Invalid boot device
#define KERNEL_BOOTDEVICE_HARDDISK  0x01        // Harddisk boot device
#define KERNEL_BOOTDEVICE_FLOPPY    0x02        // Floppy boot device
#define KERNEL_BOOTDEVICE_CDROM     0x03        // CDROM boot device
#define KERNEL_BOOTDEVICE_USB       0x04        // USB boot device
#define KERNEL_BOOTDEVICE_NETWORK   0x05        // Network boot device
#define KERNEL_BOOTDEVICE_SCSI      0x06        // SCSI (Small Computer System Interface) boot device

#define KERNEL_HOSTNAMELEN          63          // Maximum length of hostname
#define KERNEL_TEXTEDIT_BUFFSIZE    (4 * 1024)  // Buffer size of built-in text editor
#define KERNEL_CMDRECURSIONLIMIT    5           // Limit for command recursion level

// Enable/disable kernel graphic mode
#define KERNEL_GRAPHICMODE false

// Exit codes

#define EXIT_SUCCESS                0       // Success
#define EXIT_FAILURE                1       // General failure
#define EXIT_USAGE_ERROR            2       // Usage error
#define EXIT_UNKNOWN_ERROR          3       // Unknown error
#define EXIT_IO_ERROR               5       // I/O error
#define EXIT_INVALID_CONFIG         99      // Invalid configuration
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
#define PANIC(format, ...) \
    do { \
        extern char kernel_PanicLog[256]; extern void kernel_panic(void); \
        /* Print the panic message with the file name and line number */ \
        /* printf("Kernel panic: %s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__); */ \
        snprintf(kernel_PanicLog, 256, "%s:%d: " format, __FILE__, __LINE__, ##__VA_ARGS__); \
        kernel_panic(); while(1); \
        /* Enter an infinite loop that disables interrupts and halts the CPU */ \
        /* while (1) { asm volatile ("cli\n hlt"); }; */ \
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

// Structure of CPU information table
typedef struct {
    char vendor[13];
    char brand[49];
    uint64_t frequency;
    int cores;
    int threads;
    int has_sse;
    int has_avx;
    int has_vtx;
    int has_aes;
    int has_x64;
} kernel_CPUInfo_t;

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
void*   calloc(size_t nmemb, size_t size);          // Allocates cleared memory
void*   realloc(void* allocated, size_t size);      // Reallocates memory
void    free(void* allocated);                      // Frees memory
void    memory_init(uint64_t size);                 // Initializes memory manager
char*   memory_report();                            // Writes a memory report
memory_Block_t* memory_blkInfo(void* allocated);    // Returns block information

// * Process Management

// Types

typedef int32_t pid_t;                              // Process ID type
typedef size_t  uid_t;                              // User ID type
typedef size_t  gid_t;                              // Group ID type

// Constants

#define MULTITASK_ENTCOUNT          32      // Process entry count in table
#define MULTITASK_NAMELEN           16      // Maximum length of process name

#define MULTITASK_USER_COUNTMAX     16      // Maximum user and group count in system
#define MULTITASK_USER_NAMELEN      31      // Maximum username and group length
#define MULTITASK_USER_GROUPMAX     32      // Maximum of groups a user can belong
#define MULTITASK_USER_HOMEMAX      63      // Maximum user home path length
#define MULTITASK_USER_NULL ((size_t)-1)    // Null symbol for user and groups

#define MULTITASK_USER_ROOT         0       // Root user and group ID
#define MULTITASK_USER_DAEMON       1       // Daemon user and group ID
#define MULTITASK_USER_USERS        2       // Users group ID
#define MULTITASK_USER_SUDO         3       // Sudo group ID
#define MULTITASK_USER_NOBODY       15      // Nobody and nogroup ID

// Structures

// Structure of user
typedef struct {
    char name[MULTITASK_USER_NAMELEN + 1];
    gid_t group[MULTITASK_USER_GROUPMAX];
    char home[MULTITASK_USER_HOMEMAX + 1];
    uint32_t pass; bool active;
} multitask_User_t;

// Structure of process context for save/restore process registers
typedef struct {
    uint32_t
        EAX, EBX, ECX, EDX,
        ESI, EDI, ESP, EBP,
        EIP, EFLAGS, CR3;
} multitask_Context_t;

// Structure of process
typedef struct {
    bool active; uid_t user; pid_t parent;
    char name[MULTITASK_NAMELEN];
    multitask_Context_t context;
} multitask_Process_t;

// Public functions

void    multitask_init();                           // Initializes multitasking system
pid_t   spawn(char* name, void* program);           // Spawns a process
int     kill(pid_t pid);                            // Kills a process
void    yield();                                    // Switch next process
void    exit();                                     // End current process
int     uadd(char* name);                           // Add new user
int     udel(char* name);                           // Delete a user
int     upwd(char* name, char* oldp, char* newp);   // Change user password
int     uswi(char* name, char* pass);               // Switch another user
pid_t   getpid();                                   // Get current process ID
pid_t   getppid();                                  // Get current parent process ID
uid_t   getuid();                                   // Get current user ID
gid_t   getgid();                                   // Get current group ID
char* multitask_report();

// * System Call Management

// Constants

// System calls
#define SYS_EXIT        0x01                        // End current process
#define SYS_SPAWN       0x02                        // Spawn a process
#define SYS_READ        0x03                        // Read data from a specific file descriptor
#define SYS_WRITE       0x04                        // Write data to specific file descriptor
#define SYS_OPEN        0x05                        // Open a file descriptor
#define SYS_CLOSE       0x06                        // Close a file descriptor
#define SYS_YIELD       0x9E                        // Switch to next process

// File descriptors
#define STDIN           0                           // Standard input
#define STDOUT          1                           // Standard output
#define STDERR          2                           // Standard error output
#define TYPEFD          3                           // File descriptor type symbol

// Opening flags
#define O_RDONLY        (1 << 0)                    // Open file as read only
#define O_WRONLY        (1 << 1)                    // Open file as write only
#define O_RDWR          (1 << 2)                    // Open file as both reading and writing
#define O_CREAT         (1 << 3)                    // Create file if it doesn't exists
#define O_EXCL          (1 << 4)                    // Return error if file exists
#define O_TRUNC         (1 << 5)                    // Truncate file if exists
#define O_APPEND        (1 << 6)                    // All writes to file will be appended to end

// Public functions

void    syscall_init();                             // Initializes system call manager
size_t  read(int fd, void* buf, size_t count);      // Read data from a specific file descriptor
size_t  write(int fd, void* buf, size_t count);     // Write data to specific file descriptor
int     open(char* path, int flags);                // Open a file descriptor
int     close(int fd);                              // Close a file descriptor