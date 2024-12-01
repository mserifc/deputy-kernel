#include "gdt.h"

// Declare an array of GDT Entries.
struct gdt_Entry gdt_entry[3]; // Null, Code, Data

// Declare a GDT Pointer to store the base and limit of the GDT
struct gdt_Pointer gdt_ptr;

// Function for set Specific GDT Entry
void gdt_setEntry(
    int num,                // Index of the entry in the GDT
    uint32_t base,          // The starting address of the segment
    uint32_t limit,         // The size of the segment
    uint8_t access,         // Access flags (permissions for the segment)
    uint8_t granularity     // Granularity setting for segment size
) {
    gdt_entry[num].base_low    = (base & 0xFFFF);           // Set the lower 16 bits of the base address for the segment
    gdt_entry[num].base_middle = (base >> 16) & 0xFF;       // Set the middle 8 bits of the base address for the segment
    gdt_entry[num].base_high   = (base >> 24) & 0xFF;       // Set the upper 8 bits of the base address for the segment

    gdt_entry[num].limit_low   = (limit & 0xFFFF);          // Set the lower 16 bits of the segment size (limit)
    gdt_entry[num].granularity = (limit >> 16) & 0x0F;      // Set the granularity, which defines the segment's size scaling

    gdt_entry[num].granularity |= (granularity & 0xF0);     // Combine the granularity with the high nibble of the provided granularity
    gdt_entry[num].access      = access;                    // Set the access flags (defines the segment's permissions)
}

// Function for initialize the Global Descriptor Table (GDT)
void gdt_init() {
    gdt_ptr.limit = (sizeof(struct gdt_Entry) * 3) - 1;  // Set the size of the GDT (total size of all entries minus one)
    gdt_ptr.base = (uint32_t)&gdt_entry;                // Set the base address of the GDT (the address of gdt_entry array)

    // Set up the Null Segment (index 0), which is not used in practice
    gdt_setEntry(0, 0, 0, 0, 0);
    // Set up the Code Segment (index 1) with a base address of 0, 4GB size, access flags, and granularity
    gdt_setEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    // Set up the Data Segment (index 2) with a base address of 0, 4GB size, access flags, and granularity
    gdt_setEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Load the GDT into the CPU by passing the address of the GDT Pointer structure
    gdt_flush((uint32_t)&gdt_ptr);
}
