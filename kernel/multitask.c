#include "kernel.h"

bool multitask_Initialized = false;
bool multitask_Running = false;

struct multitask_Process* multitask_ProcessTable;

size_t multitask_ProcessTableSize = MULTITASK_MAX_PROCESS_LIMIT * sizeof(struct multitask_Process);

int multitask_startProcess(void* program_buffer, size_t memory_size) {
    if (
        !multitask_Initialized ||
        program_buffer == NULL ||
        memory_size == 0
    ) { return -1; }
    int pid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (!multitask_ProcessTable[i].active) {
            pid = i; break;
        }
    }
    if (pid == 0) { return -1; }
    asm volatile ("cli");
    multitask_ProcessTable[pid].active = true;
    multitask_ProcessTable[pid].inst = 0;
    multitask_ProcessTable[pid].size = memory_size;
    multitask_ProcessTable[pid].base = program_buffer;
    multitask_Running = true;
    printf("PID %d, %d\n", pid, multitask_ProcessTable[pid].active);
    // asm volatile ("sti");
    return pid;
}

int multitask_endProcess(uint32_t process_id) {
    if (
        !multitask_Initialized ||
        process_id == 1 ||
        process_id <= 0 ||
        process_id >= MULTITASK_MAX_PROCESS_LIMIT
    ) { return -1; }
    asm volatile ("cli");
    multitask_ProcessTable[process_id].active = false;
    multitask_ProcessTable[process_id].inst = 0;
    multitask_ProcessTable[process_id].size = 0;
    multitask_ProcessTable[process_id].base = NULL;
    // memset(&multitask_ProcessTable[process_id], 0, sizeof(struct multitask_Process));
    asm volatile ("sti");
    return 0;
}

void multitask_handler(void) {
    size_t instptr = (size_t)__builtin_return_address(0);
    if (!multitask_Initialized || !multitask_Running) { return; }
    // printf("Old instruction address: %d\n", instptr);
    asm volatile ("cli");
    uint32_t current_pid = 0, next_pid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (
            multitask_ProcessTable[i].active &&
            (size_t)multitask_ProcessTable[i].base <= instptr &&
            (size_t)multitask_ProcessTable[i].base + multitask_ProcessTable[i].size > instptr
        ) { current_pid = i; break; }
    }
    // printf("Current PID: %d\n", current_pid);
    if (current_pid == 0) {
        if (multitask_ProcessTable[1].inst >= multitask_ProcessTable[1].size)
            { kernel_panic("Tasks are confused and the kernel doesn't know what to do"); }
        asm volatile (
            "sti\t\n"
            "jmp *%0\t\n"
            :
            : "r"((size_t)multitask_ProcessTable[1].base + multitask_ProcessTable[1].inst)
        );
    } else {
        multitask_ProcessTable[current_pid].inst = instptr - (size_t)multitask_ProcessTable[current_pid].base;
        for (int i = current_pid + 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
            if (multitask_ProcessTable[i].active) { next_pid = i; break; }
        }
        // printf("Next PID: %d\n", next_pid);
        if (next_pid == 0) { next_pid = 1; }
        if (multitask_ProcessTable[next_pid].inst >= multitask_ProcessTable[next_pid].size) {
            kernel_panic("Tasks are confused and the kernel doesn't know what to do");
        }
        // printf("Next PID: %d\n", next_pid);
        asm volatile (
            "sti\t\n"
            "jmp *%0\t\n"
            :
            : "r"((size_t)multitask_ProcessTable[next_pid].base + multitask_ProcessTable[next_pid].inst)
        );
    }
    asm volatile ("sti");
}

int multitask_init() {
    if (multitask_Initialized) { return -1; }
    multitask_ProcessTable = (struct multitask_Process*)malloc(multitask_ProcessTableSize);
    if (multitask_ProcessTable == NULL) { kernel_panic("Multitasking initialization failed, cannot allocate memory"); }
    memset(multitask_ProcessTable, 0, multitask_ProcessTableSize);
    asm volatile ("cli");
    uint32_t divisor = MULTITASK_TIMER_PIT_FREQUENCY / MULTITASK_TIMER_FREQUENCY;
    port_outb(MULTITASK_TIMER_CTRL, 0x36);
    port_outb(MULTITASK_TIMER_COUNTER0, divisor & 0xFF);
    port_outb(MULTITASK_TIMER_COUNTER0, (divisor >> 8) & 0xFF);
    interrupts_IDTSetGate(INTERRUPTS_PIC_MASTER_OFFSET + INTERRUPTS_IRQ_TIMER, (size_t)multitask_handler);
    interrupts_PICIRQEnable(INTERRUPTS_IRQ_TIMER);
    asm volatile ("sti");
    multitask_Initialized = true;
    return 0;
}

void syscall_exit(uint32_t error_code) {
    size_t instptr = (size_t)__builtin_return_address(0);
    uint32_t pid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (
            multitask_ProcessTable[i].active &&
            (size_t)multitask_ProcessTable[i].base <= instptr &&
            (size_t)multitask_ProcessTable[i].base + multitask_ProcessTable[i].size > instptr
        ) { pid = i; break; }
    }
    if (
        pid <= 0 ||
        pid >= MULTITASK_MAX_PROCESS_LIMIT
    ) {
        return;
    } else if (pid == 1) {
        kernel_panic("Someone attempted to stop the main task");
    } else {
        multitask_endProcess(pid);
    }
}

void syscall_exit_init() {
    syscall_addEntry(SYS_EXIT, (size_t)syscall_exit);
}