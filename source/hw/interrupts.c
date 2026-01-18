#include "hw/interrupts.h"

#include "kernel.h"

// * Imported exception gates

extern void interrupts_exception0x00(void);             // Division Error
extern void interrupts_exception0x01(void);             // Debug
extern void interrupts_exception0x02(void);             // Non-maskable Interrupt
extern void interrupts_exception0x03(void);             // Breakpoint
extern void interrupts_exception0x04(void);             // Overflow
extern void interrupts_exception0x05(void);             // Bound Range Exceeded
extern void interrupts_exception0x06(void);             // Invalid Opcode
extern void interrupts_exception0x07(void);             // Device Not Available
extern void interrupts_exception0x08(void);             // Double Fault
extern void interrupts_exception0x09(void);             // Coprocessor Segment Overrun
extern void interrupts_exception0x0A(void);             // Invaild TSS
extern void interrupts_exception0x0B(void);             // Segment Not Present
extern void interrupts_exception0x0C(void);             // Stack-Segment Fault
extern void interrupts_exception0x0D(void);             // General Protection Fault
extern void interrupts_exception0x0E(void);             // Page Fault
extern void interrupts_exception0x0F(void);             // Unknown
extern void interrupts_exception0x10(void);             // x87 Floating-Point Exception
extern void interrupts_exception0x11(void);             // Alignment Check
extern void interrupts_exception0x12(void);             // Machine Check
extern void interrupts_exception0x13(void);             // SIMD Floating-Point Exception
extern void interrupts_exception0x14(void);             // Virtualization Exception
extern void interrupts_exception0x15(void);             // Control Protection Exception
extern void interrupts_exception0x16(void);             // Unknown
extern void interrupts_exception0x17(void);             // Unknown
extern void interrupts_exception0x18(void);             // Unknown
extern void interrupts_exception0x19(void);             // Unknown
extern void interrupts_exception0x1A(void);             // Unknown
extern void interrupts_exception0x1B(void);             // Unknown
extern void interrupts_exception0x1C(void);             // Hypervisor Injection Exception
extern void interrupts_exception0x1D(void);             // VMM Communication Exception
extern void interrupts_exception0x1E(void);             // Security
extern void interrupts_exception0x1F(void);             // Unknown
extern void interrupts_exceptionInterruptsInit(void);   // Exception interrupts initializer

// * Types and structures

// IDT Entry Structure
// This structure defines the entry of the Interrupt Descriptor Table (IDT).
typedef struct {
    uint16_t offset_low;            // The lower 16 bits of the interrupt handler address
    uint16_t selector;              // The segment selector
    uint8_t zero;                   // Reserved, should be set to 0
    uint8_t attributes;             // Descriptor attributes
    uint16_t offset_high;           // The higher 16 bits of the interrupt handler address
} PACKED interrupts_IDTEntry_t;     // 'packed' ensures no padding is added

// IDT Pointer Structure
// This structure defines the pointer of the Interrupt Descriptor Table (IDT).
typedef struct {
    uint16_t limit;                 // The size of the IDT (in bytes)
    uint32_t base;                  // The base address of the IDT
} PACKED interrupts_IDTPointer_t;   // 'packed' ensures no padding is added

// * Variables and tables

bool interrupts_InitLock = false;       // Initialize lock for prevent re-initializing interrupts

// Define an array of 256 IDT Entries, one for each interrupt/exception vector
interrupts_IDTEntry_t interrupts_IDTEntry[256];

// Define the IDT Pointer
interrupts_IDTPointer_t interrupts_IDTPointer;

// List of exception messages to be displayed for each exception code
static char* interrupts_exceptionMessage[] = {
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
    "Fatal: Machine Check",                 // 18
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

// * Subfunctions

// Default interrupt handler if no handler is set
void interrupts_defaultHandler(void) {
    printf("Interrupt Manager: Unknown interrupt triggered!\n");    // Print a message for an unknown interrupt
}

// The exception handler that gets called for each interrupt
NORETURN void interrupts_exceptionHandler(uint8_t num) {
    if (num > 0x1F) {   // If the exception is unknown (greater than 0x1F)
        PANIC("Manually triggered exception");
    } else {            // Else, print the exception message
        PANIC(
            "%s, Code %d",
            interrupts_exceptionMessage[num],   // Exception message
            num                                 // Exception code
        );
    }
}

// * Functions

/**
 * @brief Function for set gate in the IDT for a specific interrupt
 * 
 * @param num Interrupt vector number
 * @param handler Interrupt service handler
 */
void interrupts_setGate(int num, size_t handler) {
    if (num < 0 || num >= 256 || !interrupts_InitLock) { return; }          // Prevent invalid vectors and uninit
    size_t addr = handler ? handler : (size_t)interrupts_defaultHandler;    // Set address as default if NULL
    interrupts_IDTEntry[num].offset_low   = addr & 0xFFFF;                  // Low 16 bits of the handler
    interrupts_IDTEntry[num].selector     = 0x08;                           // Segment selector
    interrupts_IDTEntry[num].zero         = 0x00;                           // Unused
    interrupts_IDTEntry[num].attributes   = 0x8E;                           // Interrupt gate
    interrupts_IDTEntry[num].offset_high  = (addr >> 16) & 0xFFFF;          // High 16 bits of the handler
}

/**
 * @brief Function for initialize interrupt manager
 */
void interrupts_init() {
    if (interrupts_InitLock) { return; }                                        // Prevent re-initializing
    interrupts_InitLock = true;                                                 // Lock the initializer
    // Set default handler for all interrupt vectors
    for (int i = 0; i < 256; ++i) { interrupts_setGate(i, (size_t)interrupts_defaultHandler); }
    interrupts_exceptionInterruptsInit();                                       // Initialize exception handler
    interrupts_IDTPointer.limit = (sizeof(interrupts_IDTEntry_t) * 256) - 1;    // Set the IDT size
    interrupts_IDTPointer.base = (size_t)&interrupts_IDTEntry;                  // Set the IDT base address
    asm volatile ("lidtl (%0)" : : "r" (&interrupts_IDTPointer));               // Load the IDT into the IDTR register
    asm volatile (                                                              // Allow machine check exceptions
        "mov %%cr4, %%eax\t\n"                                                  // Get CR4 register
        "or $0x40, %%eax\t\n"                                                   // Set MCE bit (bit 6)
        "mov %%eax, %%cr4\t\n"                                                  // Load CR$ register
        : : : "eax"                                                             // EAX register clobbered
    );
}