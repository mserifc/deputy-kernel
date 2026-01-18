#pragma once

#include "kernel.h"
#include "platform/i386/port.h"
#include "platform/i386/interrupts.h"
#include "common.h"

#define MULTITASK_MAX_PROCESS_LIMIT 1024

#define MULTITASK_TIMER_CTRL 0x43
#define MULTITASK_TIMER_COUNTER0 0x40
#define MULTITASK_TIMER_PIT_FREQUENCY 1193182
#define MULTITASK_TIMER_FREQUENCY 100

struct multitask_Process_t {
    bool active;
    bool focus;
    size_t inst;
    size_t size;
    void* base;
};

int multitask_restart();

int multitask_switchProcess(uint32_t pid);
int multitask_switchNextProcess();

int multitask_startProcess(void* source, size_t memory);
int multitask_endProcess(uint32_t pid);

int multitask_findProcessID(size_t address);

int multitask_init();