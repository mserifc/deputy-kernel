#pragma once

#include "types.h"

// * Functions

void interrupts_setGate(int num, size_t handler);   // Set a gate in the IDT for a specific interrupt
void interrupts_init(void);                         // Initialize interrupt system