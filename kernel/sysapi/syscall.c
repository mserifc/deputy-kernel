#include "sysapi/syscall.h"

// System call table
size_t syscall_Table[SYSCALL_ENTRY_COUNT];

// Return address
size_t syscall_ReturnAddress = 0x0000;

// Function for get return address
size_t syscall_getReturnAddr() { return syscall_ReturnAddress; }

// System call handler
void syscall_handler(void) {
    syscall_ReturnAddress = (size_t)__builtin_return_address(0);
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
    if (syscall_Table[eax] != 0x00) {
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

// Function for initialize system call manager
void syscall_init() {
    memset(syscall_Table, 0, sizeof(size_t) * SYSCALL_ENTRY_COUNT);
    interrupts_IDTSetGate(SYSCALL_INTERRUPT_VECTOR, (size_t)syscall_handler);

    syscall_stdio_init();
}

// Function for add system call entry
int syscall_addEntry(int num, size_t handler) {
    if (num >= 0 && num < SYSCALL_ENTRY_COUNT) {
        syscall_Table[num] = handler;
    }
    return -1;
}