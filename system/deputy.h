#pragma once

/* ---- Types ---- */

typedef char int8_t;            // 8 bit integer type
typedef short int16_t;          // 16 bit integer type
typedef int int32_t;            // 32 bit integer type
typedef long long int64_t;      // 64 bit integer type

#define SCHAR_MIN (-128)            // Minimum value of signed char
#define SCHAR_MAX 127               // Maximum value of signed char
#define SHRT_MIN (-32768)           // Minimum value of signed short
#define SHRT_MAX 32767              // Maximum value of signed short
#define INT_MIN (-2147483647 - 1)   // Minimum value of signed integer
#define INT_MAX 2147483647          // Maximum value of signed integer

typedef unsigned char uint8_t;          // Unsigned 8 bit integer type
typedef unsigned short uint16_t;        // Unsigned 16 bit integer type
typedef unsigned int uint32_t;          // Unsigned 32 bit integer type
typedef unsigned long long uint64_t;    // Unsigned 64 bit integer type

#define UCHAR_MIN 0             // Minimum value of unsigned char
#define UCHAR_MAX 255U          // Maximum value of unsigned char
#define USHRT_MIN 0             // Minimum value of unsigned short
#define USHRT_MAX 65535U        // Maximum value of unsigned short
#define UINT_MIN 0              // Minimum value of unsigned integer
#define UINT_MAX 4294967295U    // Maximum value of unsigned integer

typedef unsigned int size_t;    // Size type for represent sizes and memory offsets

typedef void (*func_t)(void);   // Function type for represent function address

typedef unsigned char bool;     // Boolean type for represent true/false values

#define true 1      // True macro for use with boolean values
#define false 0     // False macro for use with boolean values

#define NULL ((void*)0)     // Null pointer for indicating no valid memory address

#define STRINGIFY(x) #x             // Macro for convert macro name to ASCII string
#define STRING(x) STRINGIFY(x)      // Macro for convert macro value to ASCII string
#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))    // Aligns x to a multiple of a
#define NAKED __attribute__((naked))                // No prologue or epilogue in the function
#define PACKED __attribute__((packed))              // No padding between structure members
#define NORETURN __attribute__((noreturn))          // Function does not return
#define INTERRUPT __attribute__((interrupt))        // Marks a function as an interrupt handler
#define GENERALREGSONLY __attribute__((target("general-regs-only"))) // Function uses general registers only (recommended using with INTERRUPT attribute)
#define UNUSED __attribute__((unused))              // Marks a function or variable as unused
#define USED __attribute__((used))                  // Marks a function or variable as used
#define ALIGNED(x) (__attribute__((aligned(x))))    // Align a structure to x bytes

#define va_list __builtin_va_list                       // Type definition for variable argument list
#define va_start(ap, last) __builtin_va_start(ap, last) // Initialize variable argument list
#define va_arg(ap, type) __builtin_va_arg(ap, type)     // Retrieve the next argument of a specified type
#define va_end(ap) __builtin_va_end(ap)                 // Clean up the variable argument list

/* ---- System calls ---- */

#define SYS_EXIT        0x01                        // End current process
#define SYS_EXEC        0x02                        // Execute a program
#define SYS_READ        0x03                        // Read data from a specific file descriptor
#define SYS_WRITE       0x04                        // Write data to specific file descriptor
#define SYS_OPEN        0x05                        // Open a file descriptor
#define SYS_CLOSE       0x06                        // Close a file descriptor
#define SYS_REMOVE      0x0A                       // Remove a file or empty directory
#define SYS_MKNOD       0x0E                        // Create a special file (FIFO supported only)
#define SYS_LSEEK       0x13                        // Move file descriptor pointer
#define SYS_MKDIR       0x27                       // Create a directory
#define SYS_RMDIR       0x28                       // Remove a directory
#define SYS_MAPMEM      0x5A                        // Map a memory field or file data
#define SYS_UNMAPMEM    0x5B                        // Unmap memory field
#define SYS_YIELD       0x9E                        // Switch to next process

/* ---- File descriptors ---- */

#define STDIN           0                           // Standard input
#define STDOUT          1                           // Standard output
#define STDERR          2                           // Standard error output
#define TYPEFD          3                           // File descriptor type

/* ---- Opening flags ---- */

#define O_RDONLY        (1 << 0)                    // Open file as read only
#define O_WRONLY        (1 << 1)                    // Open file as write only
#define O_RDWR          (1 << 2)                    // Open file as both reading and writing
#define O_CREAT         (1 << 3)                    // Create file if it doesn't exists
#define O_EXCL          (1 << 4)                    // Return error if file exists
#define O_TRUNC         (1 << 5)                    // Truncate file if exists
#define O_APPEND        (1 << 6)                    // All writes to file will be appended to end

/* ---- Seek modes ---- */

#define SEEK_SET        1                           // Seek from head of file
#define SEEK_CUR        2                           // Seek from current pointer of file
#define SEEK_END        3                           // Seek from end of file

/* ---- Access flags ---- */

#define S_IRUSR         0400    // Owner read
#define S_IWUSR         0200    // Owner write
#define S_IXUSR         0100    // Owner execute
#define S_IRGRP         040     // Group read
#define S_IWGRP         020     // Group write
#define S_IXGRP         010     // Group execute
#define S_IROTH         04      // Others read
#define S_IWOTH         02      // Others write
#define S_IXOTH         01      // Others execute

/* ---- File type flags ---- */

#define S_IFSOCK        0140000 // Socket
#define S_IFLNK         0120000 // Link
#define S_IFREG         0100000 // Regular
#define S_IFBLK         0060000 // Block device
#define S_IFDIR         0040000 // Directory
#define S_IFCHR         0020000 // Character device
#define S_IFIFO         0010000 // FIFO
#define S_IFMT          0170000 // File type bit format

/* ---- Functions ---- */

int syscall(int num, ...);
void yield(void);

int strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, int n);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
char *strcat(char *dst, const char *src);
char *strchr(const char *s, int c);
char *strstr(const char *hay, const char *needle);
char *strtok_r(char *str, const char *delim, char **saved);

void *memset(void *dst, int c, int n);
void *memcpy(void *dst, const void *src, int n);
void *memmove(void *dst, const void *src, int n);
int memcmp(const void *a, const void *b, int n);

int abs(int n);
int pow(int base, int exp);
int sqrt(int n);
int min(int a, int b);
int max(int a, int b);
int clamp(int val, int lo, int hi);

int atoi(const char *s);
// itoa: buf need to be at least 12 bytes
char *itoa(int n, char *buf);
// xtoa: buf need to be at least 9 bytes
char *xtoa(unsigned int n, char *buf);
int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
int isupper(int c);
int islower(int c);
int isspace(int c);
int toupper(int c);
int tolower(int c);

void putchar(const char chr);
void puts(const char* str);
void printf(const char* fmt, ...);
int snprintf(char *buffer, size_t size, const char *fmt, ...);


/* ---- Display driver ---- */

#define DISPLAY_OP_DOT  1
#define DISPLAY_OP_RECT 2
#define DISPLAY_OP_LINE 3
#define DISPLAY_OP_TRI  4
#define DISPLAY_OP_TEXT 5
#define DISPLAY_OP_RAW  6
#define DISPLAY_OP_IMG  7

typedef struct {
    int width;
    int height;
    int pitch;
    int depth;
} display_Info_t;

typedef struct {
    int op;
    uint32_t c; // 0xAARRGGBB or BB GG RR AA
    int x0, y0,
        x1, y1,
        x2, y2;
} display_Pkg_t;

typedef struct {
    int width;
    int height;
} RawImageInfo_t;

void drawFlush();
int drawRect(int x, int y, int w, int h, uint32_t c);
int drawLine(int x0, int y0, int x1, int y1, uint32_t c);
int drawTri(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t c);
int drawText(int x0, int y0, const char* str, int len, int size, uint32_t c);
int drawRaw(uint32_t* buffer, int x, int y, int w, int h);
int drawImage(uint32_t* buffer, int x, int y, int w, int h);
RawImageInfo_t* loadImage(const char* path);
void unloadImage(RawImageInfo_t* img);


#define MOUSE_LEFTBTN (1 << 0)
#define MOUSE_RIGHTBTN (1 << 1)
#define MOUSE_MIDDLEBTN (1 << 2)