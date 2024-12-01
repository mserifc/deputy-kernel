#include "interrupts.h"

struct interrupts_IDTEntry idt_entry[256];  // Define an array of 256 IDT Entries, one for each interrupt/exception vector
struct interrupts_IDTPointer idt_ptr;       // Define the IDT Pointer

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

// The exception handler that gets called for each interrupt
__attribute__((noreturn))
void interrupts_exceptionHandler(uint8_t num) {
    if (num > 0x1F) {   // If the exception is unknown (greater than 0x1F)
        kernel_panic("Exception: Unknown, Code Unknown");
    } else {            // Else, print the exception message
        kernel_panic(
            "Exception: %s, Code %d",
            interrupts_exceptionMessage[num],   // Exception message
            num                                 // Exception code
        );
    }
}

// Default interrupt handler if no handler is set
void interrupts_defaultHandler(void) {
    printf("Unknown Interrupt Triggered!");     // Print a message for an unknown interrupt
}

// Send End-of-Interrupt (EOI) signal to the PIC
void interrupts_PICSendEOI(uint8_t irq) {
	port_outb(PIC1_CMD, PIC_EOI);       // Send EOI to Master PIC
	if(irq >= 8) {                      // If the IRQ is above 7, also send EOI to Slave PIC
		port_outb(PIC2_CMD, PIC_EOI);   // Send EOI to Slave PIC
    }
}

// Read the Interrupt Request Register (IRR) from the PIC
uint16_t interrupts_PICGetIRR(void) {
    port_outb(PIC1_CMD, PIC_READ_IRR);  // Request IRR from Master PIC
    port_outb(PIC2_CMD, PIC_READ_IRR);  // Request IRR from Slave PIC
    return (port_inb(PIC2_CMD) << 8) | port_inb(PIC1_CMD);  // Return the combined result
}

// Read the In-Service Register (ISR) from the PIC
uint16_t interrupts_PICGetISR(void) {
    port_outb(PIC1_CMD, PIC_READ_ISR);  // Request ISR from Master PIC
    port_outb(PIC2_CMD, PIC_READ_ISR);  // Request ISR from Slave PIC
    return (port_inb(PIC2_CMD) << 8) | port_inb(PIC1_CMD);  // Return the combined result
}

// Enable a specific IRQ on the PIC
void interrupts_PICIRQEnable(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    if (irq < 8) {          // If the IRQ is less than 8, it's on the Master PIC
        port = PIC1_DATA;
    } else {                // Otherwise, it's on the Slave PIC
        port = PIC2_DATA;
        irq -= 8;           // Adjust for the Slave PIC
    }
    value = port_inb(port) & ~(1 << irq);   // Clear Mask of IRQ by setting bit
    port_outb(port, value);                 // Write the new mask to the port
}

// Disable a specific IRQ on the PIC
void interrupts_PICIRQDisable(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    if (irq < 8) {          // If the IRQ is less than 8, it's on the Master PIC
        port = PIC1_DATA;
    } else {                // Otherwise, it's on the Slave PIC
        port = PIC2_DATA;
        irq -= 8;           // Adjust for the Slave PIC
    }
    value = port_inb(port) | (1 << irq);    // Set Mask of IRQ by setting bit
    port_outb(port, value);                 // Write the new mask to the port
}

// Disable all IRQs by masking them
void interrupts_PICDisable(void) {
    port_outb(PIC1_DATA, 0xFF);     // Mask all IRQs on Master PIC
    port_outb(PIC2_DATA, 0xFF);     // Mask all IRQs on Slave PIC
}

// Remap the PIC to new IRQ offsets
void interrupts_PICRemap(uint8_t offset1, uint8_t offset2) {
    uint8_t p1, p2;

    // Save Masks
    p1 = port_inb(PIC1_DATA);                   // Get Master PIC's Interrupt Masks
    p2 = port_inb(PIC2_DATA);                   // Get Slave PIC's Interrupt Masks

    // Start PIC (ICW1)
    port_outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4); // Master PIC
    port_ioWait();
    port_outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4); // Slave PIC
    port_ioWait();

    // Set Interrupt Vector Offsets (ICW2)
    port_outb(PIC1_DATA, offset1);              // IRQ0 - IRQ7
    port_ioWait();
    port_outb(PIC2_DATA, offset2);              // IRQ8 - IRQ15
    port_ioWait();

    // Connect Slave PIC to Master PIC (ICW3)
    port_outb(PIC1_DATA, 0x04);                 // Master PIC informs the Slave PIC is connected
    port_ioWait();
    port_outb(PIC2_DATA, 0x02);                 // Slave PIC connects to Master PIC
    port_ioWait();

    // Set Operating Mode (ICW4)
    port_outb(PIC1_DATA, ICW4_8086);            // Set to 8086 Mode
    port_ioWait();
    port_outb(PIC2_DATA, ICW4_8086);            // Set the same Mode
    port_ioWait();

    // Restore Masks
    port_outb(PIC1_DATA, p1);                   // Restore Saved Master PIC's Interrupt Masks
    port_outb(PIC2_DATA, p2);                   // Restore Saved Slave PIC's Interrupt Masks
}

// Set a gate in the IDT for a specific interrupt
void interrupts_IDTSetGate(int num, uint32_t handler) {
    idt_entry[num].offset_low   = handler & 0xFFFF;             // Low 16 bits of the handler
    idt_entry[num].selector     = 0x08;                         // Segment selector
    idt_entry[num].zero         = 0x00;                         // Unused
    idt_entry[num].attributes   = 0x8E;                         // Interrupt gate
    idt_entry[num].offset_high  = (handler >> 16) & 0xFFFF;     // High 16 bits of the handler
}

// Load the IDT into the CPU
void interrupts_IDTLoad() {
    idt_ptr.limit = (sizeof(struct interrupts_IDTEntry) * 256) - 1;        // Set the IDT size
    idt_ptr.base = (uint32_t)&idt_entry;                        // Set the IDT base address
    asm volatile ("lidtl (%0)" : : "r" (&idt_ptr));             // Load the IDT into the IDTR register
}

// Initialize interrupt system (IDT, PIC, etc.)
void interrupts_init() {

    // Set default handler for all interrupt vectors
    for (int i = 0; i < 256; ++i) {
        interrupts_IDTSetGate(i, (uint32_t)interrupts_defaultHandler);
    }

    // Initialize exception handlers
    interrupts_exceptionInterruptsInit();

    // Load the IDT
    interrupts_IDTLoad();

    // Remap the PIC to a new IRQ Offsets
    interrupts_PICRemap(0x20, 0x28);

    // Disable all IRQs initially
    interrupts_PICDisable();

    // Enable interrupts globally
    asm volatile ("sti");

}