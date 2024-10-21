#ifndef PORT_H
#define PORT_H

#include "types.h"

typedef struct {
    uint8_t (*inb)(uint16_t port);
    void (*outb)(uint16_t port, uint8_t value);
} Port;

uint8_t port_inb(uint16_t port);
void port_outb(uint16_t port, uint8_t value);

#endif // PORT_H