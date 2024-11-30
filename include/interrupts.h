#pragma once

#include "kernel.h"
#include "types.h"
#include "port.h"
#include "common.h"

#define ICW1_ICW4 0x01          // Indicates that ICW4 will be present
#define ICW1_SINGLE 0x02        // Single (cascade) mode
#define ICW1_INTERVAL4 0x04     // Call address interval 4 (8)
#define ICW1_LEVEL 0x08         // Level triggered (edge) mode
#define ICW1_INIT 0x10          // Initialization - required!

#define ICW4_8086 0x01          // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO 0x02          // Auto (normal) EOI
#define ICW4_BUF_SLAVE 0x08     // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C    // Buffered mode/master
#define ICW4_SFNM 0x10          // Special fully nested (not)

#define PIC1 0x20               // Master PIC
#define PIC2 0xA0               // Slave PIC

#define PIC1_CMD (PIC1)         // Master PIC Command Port
#define PIC1_DATA (PIC1 + 1)    // Master PIC Data Port
#define PIC2_CMD (PIC2)         // Slave PIC Command Port
#define PIC2_DATA (PIC2 + 1)    // Slave PIC Data Port

#define PIC_READ_IRR 0x0a       // Interrupt Request Register (IRR)
#define PIC_READ_ISR 0x0b       // In-Service Register (ISR)

#define PIC_EOI 0x20            // End-Of-Interrupt Command Code

// IDT Entry Structure
// This structure defines the entry of the Interrupt Descriptor Table (IDT).
struct interrupts_IDTEntry {
    uint16_t offset_low;        // The lower 16 bits of the interrupt handler address
    uint16_t selector;          // The segment selector
    uint8_t zero;               // Reserved, should be set to 0
    uint8_t attributes;         // Descriptor attributes
    uint16_t offset_high;       // The higher 16 bits of the interrupt handler address
} __attribute__((packed));      // 'packed' ensures no padding is added

// IDT Pointer Structure
// This structure defines the pointer of the Interrupt Descriptor Table (IDT).
struct interrupts_IDTPointer {
    uint16_t limit;             // The size of the IDT (in bytes)
    uint32_t base;              // The base address of the IDT
} __attribute__((packed));      // 'packed' ensures no padding is added

// Enumeration for IRQ numbers
// This enum defines the IRQ numbers used by various hardware devices.
enum interrupts_IRQTABLE {
    IRQ_TIMER           = 0x00, // Programmable Interrupt Timer Interrupt
    IRQ_KEYBOARD        = 0x01, // Keyboard Interrupt
    IRQ_CASCADE         = 0x02, // Cascade (used internally by the two PICs. never raised)
    IRQ_COM2            = 0x03, // COM2 (if enabled)
    IRQ_COM1            = 0x04, // COM1 (if enabled)
    IRQ_LPT2            = 0x05, // LPT2 (if enabled)
    IRQ_FLOPPY          = 0x06, // Floppy Disk
    IRQ_LPT1            = 0x07, // LPT1 / Unreliable "spurious" interrupt (usually)
    IRQ_RTC             = 0x08, // CMOS real-time clock (if enabled)
    IRQ_FREE_1          = 0x09, // Free for peripherals / legacy SCSI / NIC
    IRQ_FREE_2          = 0x0A, // Free for peripherals / SCSI / NIC
    IRQ_FREE_3          = 0x0B, // Free for peripherals / SCSI / NIC
    IRQ_MOUSE           = 0x0C, // PS2 Mouse
    IRQ_FPU             = 0x0D, // FPU / Coprocessor / Inter-processor
    IRQ_PRIMARY_ATA     = 0x0E, // Primary ATA Hard Disk
    IRQ_SECONDARY_ATA   = 0x0F  // Secondary ATA Hard Disk
};

typedef enum interrupts_IRQTABLE interrupts_IRQ;    // Define IRQ Type

void interrupts_exception0x00(void);
void interrupts_exception0x01(void);
void interrupts_exception0x02(void);
void interrupts_exception0x03(void);
void interrupts_exception0x04(void);
void interrupts_exception0x05(void);
void interrupts_exception0x06(void);
void interrupts_exception0x07(void);
void interrupts_exception0x08(void);
void interrupts_exception0x09(void);
void interrupts_exception0x0A(void);
void interrupts_exception0x0B(void);
void interrupts_exception0x0C(void);
void interrupts_exception0x0D(void);
void interrupts_exception0x0E(void);
void interrupts_exception0x0F(void);
void interrupts_exception0x10(void);
void interrupts_exception0x11(void);
void interrupts_exception0x12(void);
void interrupts_exception0x13(void);
void interrupts_exception0x14(void);
void interrupts_exception0x15(void);
void interrupts_exception0x16(void);
void interrupts_exception0x17(void);
void interrupts_exception0x18(void);
void interrupts_exception0x19(void);
void interrupts_exception0x1A(void);
void interrupts_exception0x1B(void);
void interrupts_exception0x1C(void);
void interrupts_exception0x1D(void);
void interrupts_exception0x1E(void);
void interrupts_exception0x1F(void);

void interrupts_exceptionInterruptsInit(void);

// Function Prototypes
__attribute__((noreturn))
void interrupts_exceptionHandler(uint8_t num);                  // Exception handler for interrupts
void interrupts_defaultHandler(void);                           // Default Interrupt Handler

// PIC related functions
void interrupts_PICSendEOI(uint8_t irq);                        // Send End-of-Interrupt (EOI) signal to the PIC
uint16_t interrupts_PICGetIRR(void);                            // Get the Interrupt Request Register (IRR) from the PIC
uint16_t interrupts_PICGetISR(void);                            // Get the In-Service Register (ISR) from the PIC

// IRQ Enable/Disable functions
void interrupts_PICIRQEnable(uint8_t irq);                      // Enable a specific IRQ on the PIC
void interrupts_PICIRQDisable(uint8_t irq);                     // Disable a specific IRQ on the PIC

// PIC control functions
void interrupts_PICDisable(void);                               // Disable the PIC
void interrupts_PICRemap(uint8_t offset1, uint8_t offset2);     // Remap the PIC to new IRQ offsets

// IDT control functions
void interrupts_IDTSetGate(int num, uint32_t handler);          // Set a gate in the IDT for a specific interrupt
void interrupts_IDTLoad();                                      // Load the IDT into the CPU

// Initialization function
void interrupts_init();                                         // Initialize interrupt system (IDT, PIC, etc.)
