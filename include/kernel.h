#pragma once

#include "types.h"

// Kernel code base and limit address (Set in linker.ld)
extern char kernel_Base, kernel_Limit;

// Constants

#define KERNEL_BUILD                32          // Kernel build version
#define KERNEL_NAME                 "Deputy"
#define KERNEL_RELEASE              "build_" NUMBER(KERNEL_BUILD)
#define KERNEL_VERSION              "Deputy Kernel Build " NUMBER(KERNEL_BUILD) " (Jul 2025)"
#define KERNEL_PLATFORM             "deputy/i386"

#define INFO(format, ...) \
    do { printf("%s: " format "\n", __func__, ##__VA_ARGS__); } while (0)

#define WARN(format, ...) \
    do { printf("%s: Warning: " format "\n", __func__, ##__VA_ARGS__); } while (0)

#define ERR(format, ...) \
    do { printf("%s: Error: " format "\n", __func__, ##__VA_ARGS__); } while (0)

// Macro function for handling kernel panic situations
#define PANIC(format, ...) \
    do { \
        /* Print the panic message with function name */ \
        printf("Kernel panic at %s: " format "\n", __func__, ##__VA_ARGS__); \
        /* Enter an infinite loop that disables interrupts and halts the CPU */ \
        while (1) { asm volatile ("cli\n hlt"); }; \
    } while (0)

// Types and structures

// Structure of CPU information table
typedef struct {
    char vendor[13];        // Vendor name
    char brand[49];         // Brand name
    uint64_t frequency;     // Current frequency
    uint32_t cores;         // Core count
    uint32_t threads;       // Thread count
    uint32_t has_tsc;       // TSC support (stable if bit 1 set)
    uint32_t has_sse;       // SSE support
    uint32_t has_avx;       // AVX support
    uint32_t has_vtx;       // VT-x support
    uint32_t has_aes;       // AES support
    uint32_t has_x64;       // x64 support
} kernel_CPUInfo_t;

// Variables and tables

extern size_t           kernel_PhysicalSize;    // Physical size of the kernel in memory
extern size_t           kernel_OSModuleSize;    // Operating system module size in memory
extern size_t           kernel_MemorySize;      // Memory size of the machine
extern kernel_CPUInfo_t kernel_CPUInfo;         // CPU information table

// * Console

void console_clear(void);
void console_print(const char* str, int len);

// * Common Utils

// Constants

#define UTILS_RTC_INDEXPORT         0x70    // RTC (Real-Time Clock) index port
#define UTILS_RTC_DATAPORT          0x71    // RTC (Real-Time Clock) data port

#define UTILS_SPLIT_TOKCOUNT        32      // Limit of token count for split function
#define UTILS_SPLIT_TOKLEN          64      // Limit of token length for split function

#define UTILS_TABLENGTH             8       // Tabulation length

#define UTILS_DISKPARTBOOTSIGN      0x80    // Partition boot signature
#define UTILS_DISKPARTTABLEOFF      0x01BE  // Partition table offset

// Types and structures

// Structure type for represent date values
typedef struct {
    uint8_t sec;        // Second
    uint8_t min;        // Minute
    uint8_t hour;       // Hour
    uint8_t day;        // Day
    uint8_t mon;        // Month
    uint16_t year;      // Year
} date_t;

// Tokens type for represent splitted string tokens
typedef char tokens_t[UTILS_SPLIT_TOKCOUNT][UTILS_SPLIT_TOKLEN];

// Structure type for represent partition entries
typedef struct {
    uint8_t bootFlag;               // 0x80 = bootable, 0x00 = not bootable
    uint8_t startHead;              // Starting head (CHS) - can be 0 if not used
    uint8_t startSector;            // Starting sector (CHS) - bits 6-7 = cylinder high bits
    uint8_t startCylinder;          // Starting cylinder (CHS)
    uint8_t systemID;               // Partition type (FAT32 LBA = 0x0C)
    uint8_t endingHead;             // Ending head (CHS)
    uint8_t endingSector;           // Ending sector (CHS) - bits 6-7 = cylinder high bits
    uint8_t endingCylinder;         // Ending cylinder (CHS)
    uint32_t relativeSector;        // Partition start (LBA), counted from disk start
    uint32_t totalSector;           // Total sectors in the partition
} PACKED DiskPartEntry_t;

// Subfunctions

uint64_t    utils_rdtsc(void);                              // Read Time Stamp Counter
uint8_t     utils_bcd2dec(uint8_t bcd);                     // Convert binary coded decimal to decimal
int         utils_oct2bin(const char *str, int len);        // Convert ASCII octal number into binary
char*       utils_itoa(int num);                            // Convert integer to ASCII string
char*       utils_xtoa(uint32_t num);                       // Convert hexadecimal integer to ASCII string
void        utils_readRTC(                                  // Read Real-Time Clock
                uint8_t* sec, uint8_t* min, uint8_t* hour,
                uint8_t* day, uint8_t* mon, uint16_t* year);
void        utils_cpuid(                                    // Get CPU information by CPUID instruction
                uint32_t leaf, uint32_t subleaf,
                uint32_t* eax, uint32_t* ebx,
                uint32_t* ecx, uint32_t* edx);


// Functions

// Data manipulation algorithms

uint32_t    xorshift32(uint32_t state);                     // Convert a seed to a pseudo-random 32-bit integer
void        xorcipher(char* input, const char* key);        // Encrypt or decrypt data with a key
uint32_t    fnv1ahash(const uint8_t* data, size_t len);     // Hash the given data using FNV-1a hash algorithm

// Timing functions

void        date(date_t* base);         // Get RTC date
void        delay(uint32_t ms);         // Introduce a delay (based on CPU speed)
void        sleep(uint32_t sec);        // Sleep the system for a certain amount of time (based on RTC)

// Memory manipulation functions

void*       fill(void* ptr, char chr, size_t len);          // Fill a block of memory with specific value
char*       copy(char* dest, const char* src);              // Copy a block of memory from source to destination
void*       ncopy(void* dest, const void* src, size_t len); // Copy a block of memory from source to destination with limit

// Comparison functions

int         compare(const char* str1, const char* str2);                // Compare two strings
int         ncompare(const void* ptr1, const void* ptr2, size_t len);   // Compare two buffers with limit

// String manipulation functions

int         length(const char* str);                                // Get length of specific string
int         split(tokens_t* tok, const char* str, char deli);       // Splits a string by a deliminer
int         snprintf(char* buf, size_t len, const char* fmt, ...);  // Format and write output to a string

// String-integer conversion functions

int         atoi(const char* str);      // Convert ASCII string to integer
char*       unit(size_t size);          // Unitize byte size into human-readable format

// I/O functions

void        putchar(const char chr);                    // Print a single character to the standard output
void        puts(const char* str);                      // Print a string to the standard output
void        nputs(const char* str, int len);            // Print a string to the standard output with limit
void        printf(const char* fmt, ...);               // Print formatted output to the standard output

// * Memory Management

// Constants

#define MEMORY_BLKSIZE      (4 * 1024)          // Memory block size

// Functions

void*       malloc(size_t size);                // Allocates memory
void*       calloc(size_t nmemb, size_t size);  // Allocates cleared memory
void*       realloc(void* blk, size_t size);    // Reallocates memory
void        free(void* blk);                    // Frees memory
size_t      mavail(void);                       // Returns free space
void        memory_init(size_t size);           // Initializes memory manager

// * Core File System

// Constants

#define FS_MAX_PATHLEN          3840            // Maximum path length
#define FS_MAX_NAMELEN          256             // Maximum entry name length
#define FS_MAX_ENTCOUNT         16384           // Maximum entry count

#define FS_TYPE_FILE            0               // File type in file system
#define FS_TYPE_HARDLINK        1               // Hard link type in file system
#define FS_TYPE_SYMLINK         2               // Symbolic link type in file system
#define FS_TYPE_CHARDEV         3               // Character device type in file system
#define FS_TYPE_BLKDEV          4               // Block device type in file system
#define FS_TYPE_DIR             5               // Directory type in file system
#define FS_TYPE_FIFO            6               // Named pipe (FIFO) type in file system
#define FS_TYPE_MOUNTED         7               // Mounted file

#define FS_ROOTDIR              0               // Index of root directory

#define FS_DIRENTEND            -1              // End of directory entries

#define FS_STS_SUCCESS          0               // File system operation success
#define FS_STS_FAILURE          -1              // File system operation failure
#define FS_STS_NOTINIT          -2              // File system not initialized
#define FS_STS_ALREADYEXISTS    -3              // File/directory already exists
#define FS_STS_OUTOFMEMORY      -4              // Out of memory
#define FS_STS_PATHNOTFOUND     -5              // Path not found
#define FS_STS_NOTDIR           -6              // Not an directory
#define FS_STS_NOTFILE          -7              // Not an file
#define FS_STS_ENTRYNOTFOUND    -8              // File/directory not found
#define FS_STS_DIRNOTEMPTY      -9              // Directory not empty
#define FS_STS_FILEEXISTS       -10             // File name exists
#define FS_STS_DIREXISTS        -11             // Directory name exists
#define FS_STS_PERMDENIED       -12             // Permission denied

#define FS_PERM_READ            4               // Read permission
#define FS_PERM_WRITE           2               // Write permission
#define FS_PERM_EXEC            1               // Execute permission

// Structures

// Structure of base file system entry
typedef struct {
    char name[FS_MAX_PATHLEN];          // Name of entry
    int user;                           // Owner user
    int group;                          // Owner group
    size_t size;                        // Size of entry
    date_t ctime;                       // Create time
    date_t mtime;                       // Modify time
    date_t atime;                       // Access time
    uint16_t perm;                      // Permissions
    uint8_t type;                       // Type of entry
    uint8_t ftype;                      // File type
    uint8_t devperm;                    // Device permissions
    int mountslot;                      // Mounted slot number
} fs_Entry_t;

// Functions

int             fs_index(const char* path);                                     // Get index of specific entry
fs_Entry_t*     fs_stat(const char* path);                                      // Get specific entry stats
fs_Entry_t*     fs_parent(const char* path);                                    // Get parent directory
fs_Entry_t*     fs_dirent(int entry);                                           // Get directory entries
bool            fs_checkPerm(fs_Entry_t* ent, int uid, int gid, uint8_t perm);  // Check access perm of entry

int*            fs_readDir(const char* path);                                   // Read specific directory
int             fs_createDir(const char* path);                                 // Create a directory

char*           fs_readFile(const char* path);                                  // Read specific file
int             fs_writeFile(const char* path, size_t size, char* buf);         // Write a file

int             fs_remove(const char* path);                                    // Remove a file/directory
int             fs_bulkRemove(const char* path);                                // Remove bulk files and directories

int             corefs_init(void);                                              // Initialize core file system

// * Multitasking

// Variables

extern bool multitask_InStream;         // Active if the process stream started

// Functions

int         spawn(const char* name, func_t prog);   // Spawns a process
int         exec(const char* path);                 // Execute program from file system
int         kill(int pid);                          // Kills a process
void        yield(void);                            // Switchs to next process
void        exit(void);                             // Ends current process
void        multitask_init(void);                   // Initializes multitasking system

// * Driver manager

// Functions

void drivers_load(void* dev);       // Loads appropriate driver for target device

// * Mount manager

// Functions

int mountmgr_getSlot(void* data);
int mountmgr_freeSlot(int slot);
void mountmgr_init();

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
#define TYPEFD          3                           // File descriptor type

// Opening flags
#define O_RDONLY        (1 << 0)                    // Open file as read only
#define O_WRONLY        (1 << 1)                    // Open file as write only
#define O_RDWR          (1 << 2)                    // Open file as both reading and writing
#define O_CREAT         (1 << 3)                    // Create file if it doesn't exists
#define O_EXCL          (1 << 4)                    // Return error if file exists
#define O_TRUNC         (1 << 5)                    // Truncate file if exists
#define O_APPEND        (1 << 6)                    // All writes to file will be appended to end

// Variables

// Access file forcefully if this flag set (only for kernel components)
extern bool iocall_ForceAccess;

// Functions

void    syscall_init();                             // Initializes system call manager
size_t  read(int fd, void* buf, size_t count);      // Read data from a specific file descriptor
size_t  write(int fd, void* buf, size_t count);     // Write data to specific file descriptor
int     open(char* path, int flags);                // Open a file descriptor
int     close(int fd);                              // Close a file descriptor