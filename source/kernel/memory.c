#include "kernel.h"

// * Types and structures

// Structure of allocable memory block information
typedef struct {
    size_t count;
    bool allocated;
} memory_Block_t;

// * Variables and tables

bool memory_InitLock = false;       // Initialize lock for prevent re-initializing memory

void*           memory_Space;       // Allocable memory space base
void*           memory_Limit;       // Allocable memory space limit

memory_Block_t* memory_BlockV;      // Memory block structure
size_t          memory_BlockC;      // Memory block count

// * Functions

/**
 * @brief Function for allocate memory
 * 
 * @param size Size of memory block
 * 
 * @return Address of allocated memory block (If not found, returns null)
 */
void* malloc(size_t size) {
    if (!memory_InitLock) { return NULL; }
    if (size == 0) { return NULL; }

    // Calculate block count (ceil division)
    size_t count = (size + MEMORY_BLKSIZE - 1) / MEMORY_BLKSIZE;

    size_t base = 0;
    bool status = false;

    // Find free region
    for (size_t i = 0; i <= memory_BlockC - count; i++) {
        bool ok = true;
        for (size_t j = 0; j < count; j++) {
            if (memory_BlockV[i + j].allocated) {
                ok = false;
                break;
            }
        }
        if (ok) {
            base = i;
            status = true;
            break;
        }
    }

    if (!status) { return NULL; }

    // Mark blocks as allocated
    for (size_t i = 0; i < count; i++) {
        memory_BlockV[base + i].allocated = true;
        memory_BlockV[base + i].count = 0;  // clear old info
    }
    memory_BlockV[base].count = count;

    return (void*)((size_t)memory_Space + (base * MEMORY_BLKSIZE));
}

/**
 * @brief Function for allocate cleared memory
 */
void* calloc(size_t nmemb, size_t size) {
    if (!memory_InitLock) { return NULL; }
    size_t n = nmemb * size;
    if (n > UINT_MAX) { return NULL; }
    void* blk = malloc(n);
    if (blk == NULL) { return NULL; }
    fill(blk, 0, n);
    return blk;
}

/**
 * @brief Function for reallocate memory
 */
void* realloc(void* blk, size_t size) {
    if (!memory_InitLock || blk == NULL ||
        (size_t)blk < (size_t)memory_Space ||
        (size_t)blk >= (size_t)memory_Limit
    ) { return NULL; }
    void* newblk = malloc(size);
    if (newblk == NULL) { return NULL; }
    ncopy(newblk, blk, size);
    free(blk);
    return newblk;
}

/**
 * @brief Function for free an allocated memory block
 */
void free(void* blk) {
    if (!memory_InitLock || blk == NULL ||
        (size_t)blk < (size_t)memory_Space ||
        (size_t)blk >= (size_t)memory_Limit
    ) { return; }

    size_t num = ((size_t)blk - (size_t)memory_Space) / MEMORY_BLKSIZE;
    size_t count = memory_BlockV[num].count;

    if (count == 0) { return; }  // invalid free

    for (size_t i = 0; i < count; i++) {
        memory_BlockV[num + i].allocated = false;
        memory_BlockV[num + i].count = 0;
    }
}

/**
 * @brief Function for get available memory size
 */
size_t mavail() {
    if (!memory_InitLock) { return 0; }
    size_t result = 0;
    for (size_t i = 0; i < memory_BlockC; i++) {
        if (!memory_BlockV[i].allocated) { result += MEMORY_BLKSIZE; }
    }
    return result;
}

/**
 * @brief Function for initialize memory manager
 */
void memory_init(size_t size) {
    if (memory_InitLock) { return; }
    memory_InitLock = true;

    if (size > UINT_MAX) { PANIC("64-bit addressing not supported"); }

    size_t supblkc = size / (MEMORY_BLKSIZE + sizeof(memory_Block_t));
    supblkc -= supblkc ? 1 : 0;
    if (supblkc == 0) { PANIC("Not enough memory detected"); }

    size_t base = (size_t)&kernel_Limit + kernel_OSModuleSize;

    memory_Space = (void*)(base + (supblkc * sizeof(memory_Block_t)));
    memory_Space = (void*)(((size_t)memory_Space + MEMORY_BLKSIZE) & ~((size_t)MEMORY_BLKSIZE - 1));
    memory_Limit = (void*)((size_t)memory_Space + (supblkc * MEMORY_BLKSIZE));

    memory_BlockV = (memory_Block_t*)(base);
    memory_BlockC = supblkc;

    for (size_t i = 0; i < memory_BlockC; i++) {
        memory_BlockV[i].count = 0;
        memory_BlockV[i].allocated = false;
    }
}