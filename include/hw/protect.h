#pragma once

#include "types.h"
#include "kernel.h"

// * Types and structures

// Task State Segment (TSS) Structure
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;      // ring 0 stack pointer
    uint32_t ss0;       // ring 0 stack segment
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} PACKED protect_TSS_t;

extern protect_TSS_t protect_TSS;

// * Functions

void protect_init(size_t base, size_t limit);  // Function for initialize the GDT