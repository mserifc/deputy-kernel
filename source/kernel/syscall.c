#include "kernel.h"

#include "hw/interrupts.h"

extern void syscall_hnd();  // Include the system call handler

#define SYSCALL_INTVECTOR   0x80    // System call interrupt vector
#define SYSCALL_ENTCOUNT    256     // System call entry count

// System call table
void* syscall_Table[SYSCALL_ENTCOUNT];

// Router for yield function. Ignores interrupt pushed values from stack
NAKED void syscall_yieldRouter() {
    asm volatile (
        "call *%0\t\n"                  // Call yield
        //"push %%eax\t\n"                // Push EAX into stack
        //"mov 4(%%esp), %%eax\t\n"       // Load return address into EAX from stack
        //"mov %%eax, 12(%%esp)\t\n"      // Load EAX into two data ahead in stack
        //"pop %%eax\t\n"                 // Pop EAX value back
        //"add $8, %%esp\t\n"             // Add 8 bytes (2 data length) to ESP
        //"ret"                           // Return back
        // Normally I planned that sequence for remove EFLAGS and CS from stack
        // But I learned the IRET automatically does that, so thats unneeded now
        "iret"
        : : "r"(yield)
    );
}

/*NAKED void syscall_handler() {
    asm volatile (
        "push %%ebx\t\n"
        "mov syscall_Table, %%ebx\t\n"
        "shl $2, %%eax\t\n"
        "add %%ebx, %%eax\t\n"
        "pop %%ebx\t\n"
        "push %%edi\t\n"
        "push %%esi\t\n"
        "push %%edx\t\n"
        "push %%ecx\t\n"
        "push %%ebx\t\n"
        "call *%%eax\t\n"
        "add $20, %%esp\t\n"
        "iret\t\n"
        : : "m"(syscall_Table)
    );
}*/

/*static void** syscall_TablePtr = syscall_Table;

NAKED void syscall_handler() {
    asm volatile (
        "push %%ebx\n\t"
        "movl syscall_TablePtr, %%ebx\n\t"
        "shl $2, %%eax\n\t"
        "add %%ebx, %%eax\n\t"
        "pop %%ebx\n\t"
        "push %%edi\n\t"
        "push %%esi\n\t"
        "push %%edx\n\t"
        "push %%ecx\n\t"
        "push %%ebx\n\t"
        "call *(%%eax)\n\t"
        "add $20, %%esp\n\t"
        "iret\n\t"
        : : "m"(syscall_TablePtr)
    );
}*/

/**
 * @brief Handler for system call
 */
/*void syscall_handler(uint32_t cs, uint32_t eflags) {
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
}*/

// * Wrappers

extern void sys_exit(void);
extern void sys_yield(void);

int sys_exec(const char* path) {
    return exec((const char*)((size_t)path + kernel_UserlandBase));
}

size_t sys_read(int fd, void* buf, size_t count) {
    return read(fd, (void*)((size_t)buf + kernel_UserlandBase), count);
}

size_t sys_write(int fd, void* buf, size_t count) {
    return write(fd, (void*)((size_t)buf + kernel_UserlandBase), count);
}

int sys_open(const char* path, int flags) {
    return open((const char*)((size_t)path + kernel_UserlandBase), flags);
}

int sys_remove(const char* path) {
    return remove((const char*)((size_t)path + kernel_UserlandBase));
}

int sys_mknod(const char* path, int mode) {
    return mknod((const char*)((size_t)path + kernel_UserlandBase), mode);
}

int sys_mkdir(const char* path, int mode) {
    return mkdir((const char*)((size_t)path + kernel_UserlandBase), mode);
}

int sys_rmdir(const char* path) {
    return rmdir((const char*)((size_t)path + kernel_UserlandBase));
}

void* sys_mapmem(size_t size, int fd) {
    return (void*)((size_t)mapmem(size, fd) - kernel_UserlandBase);
}

void sys_unmapmem(void* ptr) {
    unmapmem((void*)((size_t)ptr + kernel_UserlandBase));
}

// * Functions

/**
 * @brief Function for initialize system call manager
 */
void syscall_init() {
    for (int i = 0; i < SYSCALL_ENTCOUNT; ++i) {
        syscall_Table[i] = NULL;
    }

    syscall_Table[SYS_EXIT] = sys_exit;
    syscall_Table[SYS_SPAWN] = sys_exec;
    syscall_Table[SYS_READ] = sys_read;
    syscall_Table[SYS_WRITE] = sys_write;
    syscall_Table[SYS_OPEN] = sys_open;
    syscall_Table[SYS_CLOSE] = close;
    syscall_Table[SYS_REMOVE] = sys_remove;
    syscall_Table[SYS_MKNOD] = sys_mknod;
    syscall_Table[SYS_LSEEK] = lseek;
    syscall_Table[SYS_MKDIR] = sys_mkdir;
    syscall_Table[SYS_RMDIR] = sys_rmdir;
    syscall_Table[SYS_MAPMEM] = sys_mapmem;
    syscall_Table[SYS_UNMAPMEM] = sys_unmapmem;

    interrupts_setGate(SYSCALL_INTVECTOR, (size_t)syscall_hnd, 0x08, 0xEE);
    interrupts_setGate(SYS_YIELD, (size_t)sys_yield, 0x08, 0xEE);
}