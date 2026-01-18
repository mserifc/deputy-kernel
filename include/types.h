#pragma once

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
#define UNUSED __attribute__((unused))              // Marks a function or variable as unused
#define USED __attribute__((used))                  // Marks a function or variable as used
#define ALIGNED(x) (__attribute__((aligned(x))))    // Align a structure to x bytes

#define va_list __builtin_va_list                       // Type definition for variable argument list
#define va_start(ap, last) __builtin_va_start(ap, last) // Initialize variable argument list
#define va_arg(ap, type) __builtin_va_arg(ap, type)     // Retrieve the next argument of a specified type
#define va_end(ap) __builtin_va_end(ap)                 // Clean up the variable argument list