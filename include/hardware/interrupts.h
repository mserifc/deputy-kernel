#pragma once

#include "types.h"
#include "kernel.h"
#include "hardware/port.h"
#include "utils.h"

// Structure of interrupt frame
typedef struct {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} PACKED interrupts_Frame_t;

// * Exception gates

void interrupts_exception0x00(void);            // Division Error
void interrupts_exception0x01(void);            // Debug
void interrupts_exception0x02(void);            // Non-maskable Interrupt
void interrupts_exception0x03(void);            // Breakpoint
void interrupts_exception0x04(void);            // Overflow
void interrupts_exception0x05(void);            // Bound Range Exceeded
void interrupts_exception0x06(void);            // Invalid Opcode
void interrupts_exception0x07(void);            // Device Not Available
void interrupts_exception0x08(void);            // Double Fault
void interrupts_exception0x09(void);            // Coprocessor Segment Overrun
void interrupts_exception0x0A(void);            // Invaild TSS
void interrupts_exception0x0B(void);            // Segment Not Present
void interrupts_exception0x0C(void);            // Stack-Segment Fault
void interrupts_exception0x0D(void);            // General Protection Fault
void interrupts_exception0x0E(void);            // Page Fault
void interrupts_exception0x0F(void);            // Unknown
void interrupts_exception0x10(void);            // x87 Floating-Point Exception
void interrupts_exception0x11(void);            // Alignment Check
void interrupts_exception0x12(void);            // Machine Check
void interrupts_exception0x13(void);            // SIMD Floating-Point Exception
void interrupts_exception0x14(void);            // Virtualization Exception
void interrupts_exception0x15(void);            // Control Protection Exception
void interrupts_exception0x16(void);            // Unknown
void interrupts_exception0x17(void);            // Unknown
void interrupts_exception0x18(void);            // Unknown
void interrupts_exception0x19(void);            // Unknown
void interrupts_exception0x1A(void);            // Unknown
void interrupts_exception0x1B(void);            // Unknown
void interrupts_exception0x1C(void);            // Hypervisor Injection Exception
void interrupts_exception0x1D(void);            // VMM Communication Exception
void interrupts_exception0x1E(void);            // Security
void interrupts_exception0x1F(void);            // Unknown
void interrupts_exceptionInterruptsInit(void);  // Exception interrupts initializer

// * Public functions

__attribute__((noreturn))
void interrupts_exceptionHandler(uint8_t num);          // Handler for exception interrupts
void interrupts_setGate(int num, uint32_t handler);     // Set a gate in the IDT for a specific interrupt
void interrupts_load();                                 // Load the IDT into the CPU
void interrupts_init();                                 // Initialize interrupt system