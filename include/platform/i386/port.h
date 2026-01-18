#pragma once

#include "types.h"

// Function Prototypes
uint8_t port_inb(uint16_t port);                    // Set Port Input Function (inb)
void port_outb(uint16_t port, uint8_t value);       // Set Port Output Function (outb)
void port_ioWait(void);                             // Set I/O Wait Function