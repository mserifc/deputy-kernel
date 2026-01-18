#include "hardware/port.h"

/**
 * @brief Function for get input from specific port (8-bit value version)
 * 
 * @param port Specific port
 * 
 * @return 8-bit value
 */
uint8_t port_inb(uint16_t port) {
    uint8_t value;
    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

/**
 * @brief Function for get input from specific port (16-bit value version)
 * 
 * @param port Specific port
 * 
 * @return 16-bit value
 */
uint16_t port_inw(uint16_t port) {
    uint16_t value;
    asm volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

/**
 * @brief Function for send output to specific port (8-bit value version)
 * 
 * @param port Specific port
 * @param value 8-bit value to be send
 */
void port_outb(uint16_t port, uint8_t value) {
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

/**
 * @brief Function for send output to specific port (16-bit value version)
 * 
 * @param port Specific port
 * @param value 16-bit value to be send
 */
void port_outw(uint16_t port, uint16_t value) {
    asm volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

/**
 * @brief Function for create a short delay for I/O operations
 */
void port_ioWait(void) {
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}