#include "platform/i386/port.h"

// Port Input Function (inb)
uint8_t port_inb(uint16_t port) {
    uint8_t value;
    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

// Port Output Function (outb)
void port_outb(uint16_t port, uint8_t value) {
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

// Port I/O Wait Function
void port_ioWait(void) {
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}