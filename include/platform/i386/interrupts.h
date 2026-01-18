#pragma once

#include "kernel.h"
#include "types.h"
#include "platform/i386/port.h"
#include "common.h"

// Initialization Command Words (ICW)
#define INTERRUPTS_PIC_ICW1_ICW4 0x01           // Indicates that ICW4 will be present
#define INTERRUPTS_PIC_ICW1_SINGLE 0x02         // Single (cascade) mode
#define INTERRUPTS_PIC_ICW1_INTERVAL4 0x04      // Call address interval 4 (8)
#define INTERRUPTS_PIC_ICW1_LEVEL 0x08          // Level triggered (edge) mode
#define INTERRUPTS_PIC_ICW1_INIT 0x10           // Initialization - required!

// Initialization Command Word 4 (ICW4) Options
#define INTERRUPTS_PIC_ICW4_8086 0x01           // 8086/88 (MCS-80/85) mode
#define INTERRUPTS_PIC_ICW4_AUTO 0x02           // Auto (normal) EOI
#define INTERRUPTS_PIC_ICW4_BUF_SLAVE 0x08      // Buffered mode/slave
#define INTERRUPTS_PIC_ICW4_BUF_MASTER 0x0C     // Buffered mode/master
#define INTERRUPTS_PIC_ICW4_SFNM 0x10           // Special fully nested (not)

// PIC Base Addresses
#define INTERRUPTS_PIC_MASTER 0x20              // Base address of Master PIC
#define INTERRUPTS_PIC_SLAVE 0xA0               // Base address of Slave PIC

// PIC Command and Data Ports
#define INTERRUPTS_PIC_MASTER_CMD               /* Command port for the Master PIC */ \
    (INTERRUPTS_PIC_MASTER)
#define INTERRUPTS_PIC_SLAVE_CMD                /* Command port for the Slave PIC */ \
    (INTERRUPTS_PIC_SLAVE)
#define INTERRUPTS_PIC_MASTER_DATA              /* Data port for the Master PIC */ \
    (INTERRUPTS_PIC_MASTER + 1)
#define INTERRUPTS_PIC_SLAVE_DATA               /* Data port for the Slave PIC */ \
    (INTERRUPTS_PIC_SLAVE + 1)

#define INTERRUPTS_PIC_MASTER_OFFSET 0x20       // Master PIC interrupt vector offset
#define INTERRUPTS_PIC_SLAVE_OFFSET 0x28        // Slave PIC interrupt vector offset

// PIC Interrupt Registers
#define INTERRUPTS_PIC_IRR 0x0A                 // Interrupt Request Register (IRR)
#define INTERRUPTS_PIC_ISR 0x0B                 // Interrupt Service Register (ISR)

// End-of-Interrupt (EOI)
#define INTERRUPTS_PIC_EOI 0x20                 // End-of-Interrupt command

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
enum INTERRUPTS_IRQTABLE {
    INTERRUPTS_IRQ_TIMER            = 0x00, // Programmable Interrupt Timer Interrupt
    INTERRUPTS_IRQ_KEYBOARD         = 0x01, // Keyboard Interrupt
    INTERRUPTS_IRQ_CASCADE          = 0x02, // Cascade (used internally by the two PICs. never raised)
    INTERRUPTS_IRQ_COM2             = 0x03, // COM2 (if enabled)
    INTERRUPTS_IRQ_COM1             = 0x04, // COM1 (if enabled)
    INTERRUPTS_IRQ_LPT2             = 0x05, // LPT2 (if enabled)
    INTERRUPTS_IRQ_FLOPPY           = 0x06, // Floppy Disk
    INTERRUPTS_IRQ_LPT1             = 0x07, // LPT1 / Unreliable "spurious" interrupt (usually)
    INTERRUPTS_IRQ_RTC              = 0x08, // CMOS real-time clock (if enabled)
    INTERRUPTS_IRQ_FREE_1           = 0x09, // Free for peripherals / legacy SCSI / NIC
    INTERRUPTS_IRQ_FREE_2           = 0x0A, // Free for peripherals / SCSI / NIC
    INTERRUPTS_IRQ_FREE_3           = 0x0B, // Free for peripherals / SCSI / NIC
    INTERRUPTS_IRQ_MOUSE            = 0x0C, // PS2 Mouse
    INTERRUPTS_IRQ_FPU              = 0x0D, // FPU / Coprocessor / Inter-processor
    INTERRUPTS_IRQ_PRIMARY_ATA      = 0x0E, // Primary ATA Hard Disk
    INTERRUPTS_IRQ_SECONDARY_ATA    = 0x0F  // Secondary ATA Hard Disk
};

typedef enum INTERRUPTS_IRQTABLE interrupts_irq_t;      // Define IRQ Type

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

// Function prototypes
__attribute__((noreturn))
void interrupts_exceptionHandler(uint8_t num);                  // Exception handler for interrupts
void interrupts_defaultHandler(void);                           // Default interrupt handler

// PIC related functions
void interrupts_PICSendEOI(uint8_t irq);                        // Send End-of-Interrupt (EOI) signal to the PIC
uint16_t interrupts_PICGetIRR(void);                            // Get the Interrupt Request Register (IRR) from the PIC
uint16_t interrupts_PICGetISR(void);                            // Get the In-Service Register (ISR) from the PIC

// IRQ enable/disable functions
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