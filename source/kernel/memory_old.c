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
    size_t count, base, found;
    bool status = 0;
    if (size <= 0) { return NULL; }
    else if (size <= MEMORY_BLKSIZE) { count = 1; }
    else { count = (size / MEMORY_BLKSIZE) + 1; }
    found = 0;
    for (size_t i = 0; i < memory_BlockC; ++i) {
        if (!memory_BlockV[i].allocated) {
            for (size_t j = 0; j < count; ++j) {
                if (i + j >= memory_BlockC) { return NULL; }
                if (memory_BlockV[i + j].allocated) {
                    found = 0; break;
                } else { found++; }
            }
        } if (found >= count) {
            base = i; status = true; break;
        } else { found = 0; }
    }
    if (!status) { return NULL; }
    for (size_t i = 0; i < count; ++i) {
        memory_BlockV[base + i].allocated = true;
    } memory_BlockV[base].count = count;
    return (void*)((size_t)memory_Space + (base * MEMORY_BLKSIZE));
}

/**
 * @brief Function for allocate cleared memory
 * 
 * @param nmemb Number of members
 * @param size Size of members
 * 
 * @return Address of allocated memory block (If not found, returns null)
 * 
 */
void* calloc(size_t nmemb, size_t size) {
    if (!memory_InitLock) { return NULL; }
    size_t n = nmemb * size; if (n > UINT_MAX) { return NULL; }
    void* blk = malloc(n); if (blk == NULL) { return NULL; }
    fill(blk, 0, n); return blk;
}

/**
 * @brief Function for reallocate memory
 * 
 * @param blk Allocated memory block
 * @param size New size of reallocated block
 * 
 * @return Address of reallocated memory block (If not found, returns null)
 */
void* realloc(void* blk, size_t size) {
    if (!memory_InitLock || blk == NULL ||
        (size_t)blk < (size_t)memory_Space ||
        (size_t)blk >= (size_t)memory_Limit
    ) { return NULL; }
    void* newblk = malloc(size);
    if (newblk == NULL) { return NULL; }
    ncopy(newblk, blk, size);
    free(blk); return newblk;
}

/**
 * @brief Function for free an allocated memory block
 * 
 * @param blk Allocated memory block
 */
void free(void* blk) {
    if (!memory_InitLock || blk == NULL ||
        (size_t)blk < (size_t)memory_Space ||
        (size_t)blk >= (size_t)memory_Limit
    ) { return; }
    size_t num = ((size_t)blk - (size_t)memory_Space) / MEMORY_BLKSIZE;
    size_t count = memory_BlockV[num].count;
    for (size_t i = 0; i < count; ++i) {
        memory_BlockV[num + i].allocated = false;
        memory_BlockV[num + i].count = 0;
    } blk = NULL;
}

/**
 * @brief Function for get available memory size
 * 
 * @return Available memory size in bytes
 */
size_t mavail() {
    if (!memory_InitLock) { return 0; }
    size_t result = 0; for (size_t i = 0; i < memory_BlockC; ++i) {
        if (!memory_BlockV[i].allocated) { result += MEMORY_BLKSIZE; }
    } return result;
}

/**
 * @brief Function for initialize memory manager
 * 
 * @param size Size of allocable memory field
 */
void memory_init(size_t size) {
    if (memory_InitLock) { return; } memory_InitLock = true;
    if (size > UINT_MAX) { PANIC("64-bit addressing not supported"); }
    size_t supblkc = size / (MEMORY_BLKSIZE + sizeof(memory_Block_t)); supblkc -= supblkc ? 1 : 0;
    if (supblkc <= 0) { PANIC("Not enough memory detected"); }
    memory_Space = (void*)((size_t)&kernel_Limit + (supblkc * sizeof(memory_Block_t)));
    memory_Space = (void*)(((size_t)memory_Space + MEMORY_BLKSIZE) & ((size_t)~(MEMORY_BLKSIZE - 1)));
    memory_Limit = (void*)((size_t)memory_Space + (supblkc * MEMORY_BLKSIZE));
    memory_BlockV = (memory_Block_t*)((size_t)&kernel_Limit); memory_BlockC = supblkc;
    for (size_t i = 0; i < memory_BlockC; ++i) {
        memory_BlockV[i].count = 0; memory_BlockV[i].allocated = false;
    }
}