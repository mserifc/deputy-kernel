#include "hardware/interrupts.h"

// Structures

// IDT Entry Structure
// This structure defines the entry of the Interrupt Descriptor Table (IDT).
struct interrupts_IDTEntry_t {
    uint16_t offset_low;        // The lower 16 bits of the interrupt handler address
    uint16_t selector;          // The segment selector
    uint8_t zero;               // Reserved, should be set to 0
    uint8_t attributes;         // Descriptor attributes
    uint16_t offset_high;       // The higher 16 bits of the interrupt handler address
} __attribute__((packed));      // 'packed' ensures no padding is added

// Define an array of 256 IDT Entries, one for each interrupt/exception vector
struct interrupts_IDTEntry_t interrupts_IDTEntry[256];

// IDT Pointer Structure
// This structure defines the pointer of the Interrupt Descriptor Table (IDT).
struct interrupts_IDTPointer_t {
    uint16_t limit;             // The size of the IDT (in bytes)
    uint32_t base;              // The base address of the IDT
} __attribute__((packed));      // 'packed' ensures no padding is added

// Define the IDT Pointer
struct interrupts_IDTPointer_t interrupts_IDTPointer;

// List of exception messages to be displayed for each exception code
char* interrupts_exceptionMessage[] = {
    "Division Error",                       // 0
    "Debug",                                // 1
    "Non-maskable Interrupt",               // 2
    "Breakpoint",                           // 3
    "Overflow",                             // 4
    "Bound Range Exceeded",                 // 5
    "Invalid Opcode",                       // 6
    "Device Not Available",                 // 7
    "Double Fault",                         // 8
    "Coprocessor Segment Overrun",          // 9
    "Invalid TSS",                          // 10
    "Segment Not Present",                  // 11
    "Stack-Segment Fault",                  // 12
    "General Protection Fault",             // 13
    "Page Fault",                           // 14
    "Unknown",                              // 15
    "x87 Floating-Point Exception",         // 16
    "Alignment Check",                      // 17
    "Machine Check",                        // 18
    "SIMD Floating-Point Exception",        // 19
    "Virtualization Exception",             // 20
    "Control Protection Exception",         // 21
    "Unknown",                              // 22
    "Unknown",                              // 23
    "Unknown",                              // 24
    "Unknown",                              // 25
    "Unknown",                              // 26
    "Unknown",                              // 27
    "Hypervisor Injection Exception",       // 28
    "VMM Communication Exception",          // 29
    "Security Exception",                   // 30
    "Unknown"                               // 31
};

// Private functions

// Default interrupt handler if no handler is set
void interrupts_defaultHandler(void) {
    printf("Interrupt Manager: Unknown Interrupt Triggered!\n");    // Print a message for an unknown interrupt
}

// Public functions

// The exception handler that gets called for each interrupt
__attribute__((noreturn))
void interrupts_exceptionHandler(uint8_t num) {
    if (num > 0x1F) {   // If the exception is unknown (greater than 0x1F)
        PANIC("Exception: Unknown, Code Unknown");
    } else {            // Else, print the exception message
        PANIC(
            "Exception: %s, Code %d",
            interrupts_exceptionMessage[num],   // Exception message
            num                                 // Exception code
        );
    }
}

/**
 * @brief Function for set gate in the IDT for a specific interrupt
 * 
 * @param num Interrupt vector number
 * @param handler Interrupt service handler
 */
void interrupts_setGate(int num, uint32_t handler) {
    interrupts_IDTEntry[num].offset_low   = handler & 0xFFFF;           // Low 16 bits of the handler
    interrupts_IDTEntry[num].selector     = 0x08;                       // Segment selector
    interrupts_IDTEntry[num].zero         = 0x00;                       // Unused
    interrupts_IDTEntry[num].attributes   = 0x8E;                       // Interrupt gate
    interrupts_IDTEntry[num].offset_high  = (handler >> 16) & 0xFFFF;   // High 16 bits of the handler
}

/**
 * @brief Function for load the IDT into the CPU
 */
void interrupts_load() {
    interrupts_IDTPointer.limit = (sizeof(struct interrupts_IDTEntry_t) * 256) - 1; // Set the IDT size
    interrupts_IDTPointer.base = (uint32_t)&interrupts_IDTEntry;                    // Set the IDT base address
    asm volatile ("lidtl (%0)" : : "r" (&interrupts_IDTPointer));                   // Load the IDT into the IDTR register
}

/**
 * @brief Function for initialize interrupt manager
 */
void interrupts_init() {
    // Set default handler for all interrupt vectors
    for (int i = 0; i < 256; ++i) {
        interrupts_setGate(i, (uint32_t)interrupts_defaultHandler);
    }
    interrupts_exceptionInterruptsInit();   // Initialize exception handler
    interrupts_load();                      // Load the IDT
}