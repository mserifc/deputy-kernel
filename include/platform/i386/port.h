#pragma once

#include "types.h"

// Function Prototypes

uint8_t port_inb(uint16_t port);                // 8bit port input function (inb)
uint16_t port_inw(uint16_t port);               // 16bit port input function (inw)

void port_outb(uint16_t port, uint8_t value);   // 8bit port output function (outb)
void port_outw(uint16_t port, uint16_t value);  // 16bit port output function (outw)

void port_ioWait(void);                         // Port I/O Wait Function