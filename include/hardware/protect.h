#pragma once

#include "types.h"

// * Public functions

// Function for load the GDT into CPU's GDT Pointer Register
void protect_flush(uint32_t ptr);   // This will update the CPU's GDT register with the new GDT

// Function for initialize the GDT
void protect_init();                // Initializes the GDT and sets up necessary pointers