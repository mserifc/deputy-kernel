#include "platform/i386/port.h"

// Function for send a 8bit value to specific input port (inb)
uint8_t port_inb(uint16_t port) {
    uint8_t value;
    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

// Function for send a 16bit value to specific input port (inw)
uint16_t port_inw(uint16_t port) {
    uint16_t value;
    asm volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

// Function for get a 8bit value from specific output port (outb)
void port_outb(uint16_t port, uint8_t value) {
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

// Function for get a 16bit value from specific output port (outw)
void port_outw(uint16_t port, uint16_t value) {
    asm volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

// Function for wait port i/o
void port_ioWait(void) {
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}