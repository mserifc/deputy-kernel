#include "kernel.h"
#include "hardware/interrupts.h"

#define SYSCALL_INTVECTOR   0x80    // System call interrupt vector
#define SYSCALL_ENTCOUNT    256     // System call entry count

// System call table
size_t syscall_Table[SYSCALL_ENTCOUNT];

/**
 * @brief Handler for system call
 */
void syscall_handler(void) {
    uint32_t eax, ebx, ecx, edx, esi, edi;
    asm volatile (
        "mov %%eax, %0\n\t"
        "mov %%ebx, %1\n\t"
        "mov %%ecx, %2\n\t"
        "mov %%edx, %3\n\t"
        "mov %%esi, %4\n\t"
        "mov %%edi, %5\n\t"
        : "=r"(eax),
          "=r"(ebx),
          "=r"(ecx),
          "=r"(edx),
          "=r"(esi),
          "=r"(edi)
        :
        : "memory"
    );
    if (syscall_Table[eax] != 0x0000) {
        asm volatile (
            "mov %0, %%eax\n\t"
            "mov %1, %%ebx\n\t"
            "mov %2, %%ecx\n\t"
            "mov %3, %%edx\n\t"
            "mov %4, %%esi\n\t"
            "mov %5, %%edi\n\t"
            :
            : "r"(eax),
              "r"(ebx),
              "r"(ecx),
              "r"(edx),
              "r"(esi),
              "r"(edi)
            : "memory"
        );
        int (*fn)() = (int (*)())syscall_Table[eax];
        int result = fn(ebx, ecx, edx, esi, edi);
        asm volatile ("mov %0, %%eax" : : "r"(result) : "%eax");
    }
}

/**
 * @brief Function for initialize system call manager
 */
void syscall_init() {
    for (int i = 0; i < SYSCALL_ENTCOUNT; ++i) { syscall_Table[i] = 0x0000; }
    interrupts_setGate(SYSCALL_INTVECTOR, (size_t)syscall_handler);

    interrupts_setGate(SYS_YIELD, (size_t)yield);

    syscall_Table[SYS_EXIT] = (size_t)exit;
    syscall_Table[SYS_SPAWN] = (size_t)spawn;
    syscall_Table[SYS_READ] = (size_t)read;
    syscall_Table[SYS_WRITE] = (size_t)write;
    syscall_Table[SYS_OPEN] = (size_t)open;
    syscall_Table[SYS_CLOSE] = (size_t)close;
}