#pragma once

#include "types.h"
#include "platform/i386/interrupts.h"

#define SYSCALL_INTERRUPT_VECTOR 0x80       // System call interrupt vector number
#define SYSCALL_ENTRY_COUNT 256             // System call entry count

// Enumeration for system call numbers
enum SYSCALL_NUMBERTABLE {
    SYS_EXIT = 0x01,
    SYS_READ = 0x03,
    SYS_WRITE = 0x04
};

// Function prototypes

size_t syscall_getReturnAddr();     // Get return address

void syscall_init();    // Initialize system call manager
int syscall_addEntry(int num, size_t handler);  // Add system call entry

void syscall_exit_init();
void syscall_stdio_init();