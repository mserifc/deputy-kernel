#include "hw/protect.h"

// * Imports

// Function for load the GDT into CPU's GDT Pointer Register
extern void protect_flush(uint32_t ptr);

// * Types and structures

// Global Descriptor Table (GDT) Entry Structure
// GDT entries describe memory segments (like code or data segments)
typedef struct {
    uint16_t limit_low;         // The lower 16 bits of the segment's size (limit)
    uint16_t base_low;          // The lower 16 bits of the segment's base address
    uint8_t base_middle;        // The middle 8 bits of the segment's base address
    uint8_t access;             // Access flags (defines if the segment is readable, writable, etc.)
    uint8_t granularity;        // Defines the segment's granularity (scale) and size limits
    uint8_t base_high;          // The higher 8 bits of the segment's base address
} PACKED protect_GDTEntry_t;    // 'packed' ensures no padding is added

// Global Descriptor Table (GDT) Pointer Structure
// Used to load the GDT into the CPU
typedef struct {
    uint16_t limit;             // The size of the GDT
    uint32_t base;              // The starting address of the GDT in memory
} PACKED protect_GDTPointer_t;  // 'packed' ensures no padding is added

// * Variables and tables

bool protect_InitLock = false;      // Initialize lock for prevent re-initializing protected mode

// Declare an array of GDT Entries.
protect_GDTEntry_t protect_GDTEntry[3];     // Null, Code, Data

// Declare a GDT Pointer to store the base and limit of the GDT
protect_GDTPointer_t protect_GDTPointer;

// * Subfunctions

// Function for set specific GDT Entry
void protect_setGDTEntry (
    int num,                // Index of the entry in the GDT
    uint32_t base,          // The starting address of the segment
    uint32_t limit,         // The size of the segment
    uint8_t access,         // Access flags (permissions for the segment)
    uint8_t granularity     // Granularity setting for segment size
) {
    protect_GDTEntry[num].base_low     = (base & 0xFFFF);       // Set the lower 16 bits of the base address for the segment
    protect_GDTEntry[num].base_middle  = (base >> 16) & 0xFF;   // Set the middle 8 bits of the base address for the segment
    protect_GDTEntry[num].base_high    = (base >> 24) & 0xFF;   // Set the upper 8 bits of the base address for the segment

    protect_GDTEntry[num].limit_low    = (limit & 0xFFFF);      // Set the lower 16 bits of the segment size (limit)
    protect_GDTEntry[num].granularity  = (limit >> 16) & 0x0F;  // Set the granularity, which defines the segment's size scaling

    protect_GDTEntry[num].granularity |= (granularity & 0xF0);  // Combine the granularity with the high nibble of the provided granularity
    protect_GDTEntry[num].access       = access;                // Set the access flags (defines the segment's permissions)
}

// * Functions

// Function for initialize the Global Descriptor Table (GDT)
void protect_init() {
    if (protect_InitLock) { return; }   // Prevent re-initializing
    protect_InitLock = true;            // Lock the initializer
    
    // Set the size of the GDT (total size of all entries minus one)
    protect_GDTPointer.limit = (sizeof(protect_GDTEntry_t) * 3) - 1;
    // Set the base address of the GDT (the address of gdt_entry array)
    protect_GDTPointer.base = (uint32_t)&protect_GDTEntry;

    // Set up the Null Segment (index 0), which is not used in practice
    protect_setGDTEntry(0, 0, 0, 0, 0);
    // Set up the Code Segment (index 1) with a base address of 0, 4GB size, access flags, and granularity
    protect_setGDTEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Set up the Data Segment (index 2) with a base address of 0, 4GB size, access flags, and granularity
    protect_setGDTEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // uint32_t base  = 0x00200000;
    // uint32_t limit = 0x00800000 - 1;  // 8MB

    // gdt_setEntry(3, base, limit, 0xFA, 0xCF);  // Code, ring 3 (user)

    // gdt_setEntry(4, base, limit, 0xF2, 0xCF);  // Data, ring 3 (user)

    // Load the GDT into the CPU by passing the address of the GDT Pointer structure
    protect_flush((uint32_t)&protect_GDTPointer);
}