#pragma once

#include "types.h"

// * Public functions

uint8_t     port_inb(uint16_t port);                    // Get input from specific port (8-bit value)
uint16_t    port_inw(uint16_t port);                    // Get input from specific port (16-bit value)
uint32_t    port_inl(uint16_t port);                    // Get input from specific port (32-bit value)

void        port_outb(uint16_t port, uint8_t value);    // Send output to specific port (8-bit value)
void        port_outw(uint16_t port, uint16_t value);   // Send output to specific port (16-bit value)
void        port_outl(uint16_t port, uint32_t value);   // Send output to specific port (32-bit value)

void        port_ioWait(void);                          // Create a short delay for I/O operations