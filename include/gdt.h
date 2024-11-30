#pragma once

#include "types.h"

// Global Descriptor Table (GDT) Entry Structure
// GDT entries describe memory segments (like code or data segments)
struct gdt_Entry {
    uint16_t limit_low;         // The lower 16 bits of the segment's size (limit)
    uint16_t base_low;          // The lower 16 bits of the segment's base address
    uint8_t base_middle;        // The middle 8 bits of the segment's base address
    uint8_t access;             // Access flags (defines if the segment is readable, writable, etc.)
    uint8_t granularity;        // Defines the segment's granularity (scale) and size limits
    uint8_t base_high;          // The higher 8 bits of the segment's base address
} __attribute__((packed));      // 'packed' ensures no padding is added

// Global Descriptor Table (GDT) Pointer Structure
// Used to load the GDT into the CPU
struct gdt_Pointer {
    uint16_t limit;             // The size of the GDT
    uint32_t base;              // The starting address of the GDT in memory
} __attribute__((packed));      // 'packed' ensures no padding is added

// Function Prototype for set Specific GDT Entry
void gdt_setEntry(
    int num,                    // Index of the entry in the GDT
    uint32_t base,              // The starting address of the segment
    uint32_t limit,             // The size of the segment
    uint8_t access,             // Access flags (permissions for the segment)
    uint8_t granularity         // Granularity setting for segment size
);

// Function Prototype for load the GDT into CPU's GDT Pointer Register
void gdt_flush(uint32_t ptr);   // This will update the CPU's GDT register with the new GDT

// Function Prototype for initialize the GDT
void gdt_init();                // Initializes the GDT and sets up necessary pointers
