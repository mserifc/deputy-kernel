#ifndef KERNEL_H
#define KERNEL_H

#include "common.h"

#define _kernel_panic(format, ...) \
    do { \
        printf("Kernel panic: %s:%d: " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        while(1); \
    } while (0)

typedef struct {
    void (*init)();
    void* (*allocate)();
    void (*free)(void* allocated);
    char* (*report)();
} Memory;

void memory_init();
void* memory_allocate();
void memory_free(void* allocated);
char* memory_report();

#endif // KERNEL_H