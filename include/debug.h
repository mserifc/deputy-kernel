#pragma once

// Macro for make a breakpoint interrupt
#define breakpoint() do { asm volatile ("int $0x03"); } while (0)