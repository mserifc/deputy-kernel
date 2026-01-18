#include "kernel.h"

#include "hw/interrupts.h"

#define SYSCALL_INTVECTOR   0x80    // System call interrupt vector
#define SYSCALL_ENTCOUNT    256     // System call entry count

// System call table
static void* syscall_Table[SYSCALL_ENTCOUNT];

// Router for yield function. Ignores interrupt pushed values from stack
NAKED void syscall_yieldRouter() {
    asm volatile (
        "call *%0\t\n"                  // Call yield
        "push %%eax\t\n"                // Push EAX into stack
        "mov 4(%%esp), %%eax\t\n"       // Load return address into EAX from stack
        "mov %%eax, 12(%%esp)\t\n"      // Load EAX into two data ahead in stack
        "pop %%eax\t\n"                 // Pop EAX value back
        "add $8, %%esp\t\n"             // Add 8 bytes (2 data length) to ESP
        "ret"                           // Return back
        : : "r"(yield)
    );
}

/**
 * @brief Handler for system call
 */
void syscall_handler(uint32_t cs, uint32_t eflags) {
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
        int result;
        if (syscall_Table[eax] != 0 && eax < (sizeof(syscall_Table)/sizeof(size_t))) {
            int (*fn)() = (int (*)())syscall_Table[eax];
            result = fn(ebx, ecx, edx, esi, edi);
        } else { result = -1; }
        asm volatile ("mov %0, %%eax" : : "r"(result) : "%eax");
    }
}

/**
 * @brief Function for initialize system call manager
 */
void syscall_init() {
    syscall_Table[SYS_EXIT] = exit;
    syscall_Table[SYS_SPAWN] = spawn;
    syscall_Table[SYS_READ] = read;
    syscall_Table[SYS_WRITE] = write;
    syscall_Table[SYS_OPEN] = open;
    syscall_Table[SYS_CLOSE] = close;

    interrupts_setGate(SYSCALL_INTVECTOR, (size_t)syscall_handler);
    interrupts_setGate(SYS_YIELD, (size_t)syscall_yieldRouter);
}