#include "kernel.h"

// Multitask initialize state
bool multitask_Initialized = false;

// Process table pointer
struct multitask_Process* multitask_ProcessTable;

// Process table size
size_t multitask_ProcessTableSize = MULTITASK_MAX_PROCESS_LIMIT * sizeof(struct multitask_Process);

// ! Context switch system failed and aborted
/*
__attribute__((naked))
void saveContext(struct multitask_Context* context) {
    asm volatile (
        "mov %%eax, %0\t\n"
        "mov %%ebx, %1\t\n"
        "mov %%ecx, %2\t\n"
        "mov %%edx, %3\t\n"
        "mov %%esi, %4\t\n"
        "mov %%edi, %5\t\n"
        "mov %%ebp, %6\t\n"
        "mov %%esp, %7\t\n"
        "pushf\t\n"
        "pop %8\t\n"
        "ret\t\n"
        : "=m"(context->EAX),
          "=m"(context->EBX),
          "=m"(context->ECX),
          "=m"(context->EDX),
          "=m"(context->ESI),
          "=m"(context->EDI),
          "=m"(context->EBP),
          "=m"(context->ESP),
          "=m"(context->EFLAGS)
    );
}

__attribute__((naked))
void loadContext(struct multitask_Context* context) {
    asm volatile (
        "mov %0, %%eax\t\n"
        "mov %1, %%ebx\t\n"
        "mov %2, %%ecx\t\n"
        "mov %3, %%edx\t\n"
        "mov %4, %%esi\t\n"
        "mov %5, %%edi\t\n"
        "mov %6, %%ebp\t\n"
        "mov %7, %%esp\t\n"
        "push %8\t\n"
        "popf\t\n"
        "ret\t\n"
        :
        : "m"(context->EAX),
          "m"(context->EBX),
          "m"(context->ECX),
          "m"(context->EDX),
          "m"(context->ESI),
          "m"(context->EDI),
          "m"(context->EBP),
          "m"(context->ESP),
          "m"(context->EFLAGS)
    );
}
*/

// Function for spawn a process
int spawn(void* program_addr, size_t memory_size) {
    if (
        !multitask_Initialized ||
        program_addr == NULL ||
        memory_size <= 0
    ) { return -1; }
    int pid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (!multitask_ProcessTable[i].active) { pid = i; break; }
    }
    if (pid == 0) { return -1; }
    multitask_ProcessTable[pid].active = true;
    // saveContext(&multitask_ProcessTable[pid].context);
    multitask_ProcessTable[pid].inst = 0;
    multitask_ProcessTable[pid].size = memory_size;
    multitask_ProcessTable[pid].base = program_addr;
    return pid;
}

// Function for kill a process
int kill(int pid) {
    if (
        !multitask_Initialized ||
        pid <= 1 ||
        pid >= MULTITASK_MAX_PROCESS_LIMIT
    ) { return -1; }
    multitask_ProcessTable[pid].active = false;
    memset(&multitask_ProcessTable[pid].context, 0, sizeof(struct multitask_Context));
    multitask_ProcessTable[pid].inst = 0;
    multitask_ProcessTable[pid].size = 0;
    multitask_ProcessTable[pid].base = NULL;
    return 0;
}

// Function for switch to next process
void yield() {
    if (!multitask_Initialized) { return; }
    size_t instptr = (size_t)__builtin_return_address(0);
    int current_pid = 0, next_pid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (
            multitask_ProcessTable[i].active &&
            (size_t)multitask_ProcessTable[i].base <= instptr &&
            (size_t)multitask_ProcessTable[i].base + multitask_ProcessTable[i].size > instptr
        ) { current_pid = i; break; }
    }
    if (current_pid == 0) {
        if (multitask_ProcessTable[1].inst >= multitask_ProcessTable[1].size) {
            kernel_panic("Tasks are confused and the kernel doesn't know what to do");
        }
        // loadContext(&multitask_ProcessTable[1].context);
        asm volatile (
            "jmp *%0"
            :
            : "r"((size_t)multitask_ProcessTable[1].base + multitask_ProcessTable[1].inst)
        );
    } else {
        // saveContext(&multitask_ProcessTable[current_pid].context);
        multitask_ProcessTable[current_pid].inst =
            instptr - (size_t)multitask_ProcessTable[current_pid].base;
        for (int i = current_pid + 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
            if (multitask_ProcessTable[i].active) { next_pid = i; break; }
        }
        if (next_pid == 0) { next_pid = 1; }
        if (multitask_ProcessTable[next_pid].inst >= multitask_ProcessTable[next_pid].size) {
            kernel_panic("Tasks are confused and the kernel doesn't know what to do");
        }
        // loadContext(&multitask_ProcessTable[next_pid].context);
        asm volatile (
            "jmp *%0"
            :
            : "r"((size_t)multitask_ProcessTable[next_pid].base + multitask_ProcessTable[next_pid].inst)
        );
    }
    kernel_panic("Tasks are confused and the kernel doesn't know what to do");
}

// Function for initialize multitasking system
int multitask_init() {
    if (multitask_Initialized) { return -1; }
    multitask_ProcessTable = malloc(multitask_ProcessTableSize);
    if (multitask_ProcessTable == NULL) { kernel_panic("Multitasking initialization failed, cannot allocate memory"); }
    memset(multitask_ProcessTable, 0, multitask_ProcessTableSize);
    multitask_Initialized = true;
    return 0;
}