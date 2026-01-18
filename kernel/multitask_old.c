#include "multitask.h"

bool multitask_Running = false;

struct multitask_Process_t* multitask_ProcessTable;

size_t multitask_ProcessTableSize = MULTITASK_MAX_PROCESS_LIMIT * sizeof(struct multitask_Process_t);

uint32_t multitask_LastPID = 0;

int multitask_restart() {
    asm volatile ("cli");
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        multitask_ProcessTable[i].focus = false;
        multitask_ProcessTable[i].inst = 0;
    }
    int num = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (multitask_ProcessTable[i].active) { num = i; }
    }
    if (num == 0) { return -1; }
    multitask_ProcessTable[num].focus = true;
    asm volatile ("sti");
    asm volatile ("jmp *%0" : : "r"(multitask_ProcessTable[num].base));
    return -1;
}

int multitask_switchProcess(uint32_t pid) {
    size_t instptr = (size_t)__builtin_return_address(0);
    if (pid <= 0 && pid >= MULTITASK_MAX_PROCESS_LIMIT) { return -1; }
    int oldpid = 0, newpid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (
            multitask_ProcessTable[i].active ||
            multitask_ProcessTable[i].focus
        ) {
            oldpid = i;
        }
    }
    if (oldpid == 0) { return -1; }
    if (multitask_ProcessTable[pid].focus) { kernel_panic("Exception: Switching task already in focus"); }
    if (multitask_ProcessTable[pid].active) { newpid = pid; } else { return -1; }
    asm volatile ("cli");
    if ((instptr - (size_t)multitask_ProcessTable[oldpid].base) > multitask_ProcessTable[oldpid].size) {
        asm volatile ("sti"); return -1;
    }
    multitask_ProcessTable[oldpid].inst = instptr - (size_t)multitask_ProcessTable[oldpid].base;
    multitask_ProcessTable[oldpid].focus = false;
    multitask_ProcessTable[newpid].focus = true;
    asm volatile ("sti");
    asm volatile ("jmp *%0" : : "r"((size_t)multitask_ProcessTable[newpid].base + multitask_ProcessTable[newpid].inst));
    return 0;
}

int multitask_switchNextProcess() {
    size_t instptr = (size_t)__builtin_return_address(0);
    int oldpid = 0, newpid = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (
            multitask_ProcessTable[i].active ||
            multitask_ProcessTable[i].focus
        ) {
            oldpid = i;
        }
    }
    if (oldpid == 0) { return -1; }
    // refind:
    for (int i = oldpid + 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (multitask_ProcessTable[i].focus) { kernel_panic("Exception: More than one task found in focus"); }
        if (multitask_ProcessTable[i].active) {
            newpid = i;
        }
    }
    if (newpid == 0) { return -1; }
    asm volatile ("cli");
    if ((instptr - (size_t)multitask_ProcessTable[oldpid].base) > multitask_ProcessTable[oldpid].size) {
        asm volatile ("sti"); return -1;
    }
    multitask_ProcessTable[oldpid].inst = instptr - (size_t)multitask_ProcessTable[oldpid].base;
    multitask_ProcessTable[oldpid].focus = false;
    multitask_ProcessTable[newpid].focus = true;
    asm volatile ("sti");
    asm volatile ("jmp *%0" : : "r"((size_t)multitask_ProcessTable[newpid].base + multitask_ProcessTable[newpid].inst));
    return 0;
}

int multitask_startProcess(void* source, size_t memory) {
    int num = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (!multitask_ProcessTable[i].active) {
            num = i; break;
        }
    }
    if (num == 0) { return -1; }
    asm volatile ("cli");
    multitask_ProcessTable[num].active = true;
    multitask_ProcessTable[num].focus = false;
    multitask_ProcessTable[num].inst = 0;
    multitask_ProcessTable[num].size = memory;
    multitask_ProcessTable[num].base = malloc(memory);
    if (multitask_ProcessTable[num].base == NULL) { asm volatile ("sti"); return -1; }
    memcpy(multitask_ProcessTable[num].base, source, memory);
    asm volatile ("sti");
    return 0;
}

int multitask_endProcess(uint32_t pid) {
    if (pid <= 0 && pid >= MULTITASK_MAX_PROCESS_LIMIT) { return -1; }
    asm volatile ("cli");
    if (multitask_ProcessTable[pid].base != NULL) { free(multitask_ProcessTable[pid].base); }
    memset(&multitask_ProcessTable[pid], 0, sizeof(struct multitask_Process_t));
    asm volatile ("sti");
    return 0;
}

int multitask_findProcessID(size_t address) {
    int num = 0;
    for (int i = 1; i < MULTITASK_MAX_PROCESS_LIMIT; ++i) {
        if (multitask_ProcessTable[i].active) {
            if (
                (size_t)multitask_ProcessTable[i].base <= address &&
                (size_t)multitask_ProcessTable[i].base + multitask_ProcessTable[i].size > address
            ) { num = i; }
        }
    }
    return num;
}

void multitask_handler(void) {
    // size_t* stack;
    // asm volatile ("mov %%esp, %0" : "=r"(stack));
    // size_t instptr = *(stack + 1);
    size_t instptr = (size_t)__builtin_return_address(0);
    printf("Old instruction address: %d\n", instptr);
    // asm volatile ("jmp *%0" : : "r"(instptr));
    multitask_switchNextProcess();
}

int multitask_init() {
    if (multitask_Running) { return -1; }
    multitask_ProcessTable = (struct multitask_Process_t*)malloc(multitask_ProcessTableSize);
    memset(multitask_ProcessTable, 0, multitask_ProcessTableSize);
    asm volatile ("cli");
    uint32_t divisor = MULTITASK_TIMER_PIT_FREQUENCY / MULTITASK_TIMER_FREQUENCY;
    port_outb(MULTITASK_TIMER_CTRL, 0x36);
    port_outb(MULTITASK_TIMER_COUNTER0, divisor & 0xFF);
    port_outb(MULTITASK_TIMER_COUNTER0, (divisor >> 8) & 0xFF);
    interrupts_IDTSetGate(INTERRUPTS_PIC_MASTER_OFFSET + INTERRUPTS_IRQ_TIMER, (size_t)multitask_handler);
    interrupts_PICIRQEnable(INTERRUPTS_IRQ_TIMER);
    asm volatile ("sti");
    multitask_Running = true;
    return 0;
}