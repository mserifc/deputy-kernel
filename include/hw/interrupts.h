#pragma once

#include "types.h"

// * Types and structures

// Structure of interrupt frame for interrupt service routines
typedef struct {
    uint32_t ip;    // Instruction Pointer
    uint32_t cs;    // Code Segment
    uint32_t flags; // EFLAGS
    uint32_t sp;    // Stack Pointer (pushed if privilege changed)
    uint32_t ss;    // Stack Segment (pushed if privilege changed)
} interrupts_Frame_t;

// * Functions

    /**
     * Note:
     * for selector, 0x08 is for kernel segment (IRQ need to be in kernel segment)
     * for selector, 0x1B is for userland segment (we never used it)
     * for flags, 0x8E is for normal exception/IRQ handler
     * for flags, 0xEE is for system call
     */

void interrupts_setGate(int num, size_t hnd, uint16_t sel, uint8_t flags);   // Set a gate in the IDT for a specific interrupt
void interrupts_init(void);                         // Initialize interrupt system