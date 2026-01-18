#include "kernel.h"

// * Variables

// Allocable memory space pointer
void* memory_Space;

// Allocable memory size
size_t memory_Size;

// Memory block list base area
char memory_BlockBase[12 * 1024 * 1024];

// Memory block list structure
memory_Block_t* memory_Block;

// Memory block count
uint32_t memory_BlockCount;

// Memory report message buffer
char memory_ReportMessage[256];

// * Public functions

/**
 * @brief Function for allocate memory
 * 
 * @param size Size of memory block
 * 
 * @return Address of allocated memory block (If not found, returns null)
 */
void* malloc(size_t size) {
    size_t count, base, found;
    bool status = 0;
    if (size == 0) {
        return NULL;
    } else if (size > MEMORY_BLOCKSIZE) {
        count = (size / MEMORY_BLOCKSIZE) + 1;
    } else {
        count = 1;
    }
    found = 0;
    for (size_t i = 0; i < memory_BlockCount; ++i) {
        if (!memory_Block[i].allocated) {
            for (size_t j = 0; j < count; ++j) {
                if (i + j >= memory_BlockCount) { return NULL; }
                if (memory_Block[i + j].allocated) {
                    found = 0;
                    break;
                } else {
                    found++;
                }
            }
        }
        if (found >= count) {
            base = i;
            status = true;
            break;
        } else {
            found = 0;
        }
    }
    if (!status) { return NULL; }
    for (size_t i = 0; i < count; ++i) {
        memory_Block[base + i].allocated = true;
    }
    memory_Block[base].count = count;
    // fill(memory_Block[base].entry, 0, memory_Block[base].count * MEMORY_BLOCKSIZE);
    return memory_Block[base].entry;
}

/**
 * @brief Function for allocate cleared memory
 * 
 * @param size Size of memory block
 * 
 * @return Address of cleared allocated memory block (If not found, returns null)
 * 
 * ! NOTE: This function will be designed from scratch
 */
void* calloc(size_t size) {
    void* blk = malloc(size);
    if (blk == NULL) { return NULL; }
    fill(blk, 0, memory_blkInfo(blk)->count * MEMORY_BLOCKSIZE);
    return blk;
}

/**
 * @brief Function for reallocate memory
 * 
 * @param allocated Allocated memory block
 * @param size New size of reallocated block
 * 
 * @return Address of reallocated memory block (If not found, returns null)
 */
void* realloc(void* allocated, size_t size) {
    if (
        allocated == NULL ||
        (size_t)allocated < (size_t)memory_Space ||
        (size_t)allocated >= (size_t)memory_Space + memory_Size
    ) { return NULL; }
    void* newblk = malloc(size);
    if (newblk == NULL) { return NULL; }
    ncopy(newblk, allocated, size);
    free(allocated);
    return newblk;
}

/**
 * @brief Function for free an allocated memory block
 * 
 * @param allocated Allocated memory block
 */
void free(void* allocated) {
    if (
        allocated == NULL ||
        (size_t)allocated < (size_t)memory_Space ||
        (size_t)allocated >= (size_t)memory_Space + memory_Size
    ) { return; }
    size_t num = ((size_t)allocated - (size_t)memory_Space) / MEMORY_BLOCKSIZE;
    size_t count = memory_Block[num].count;
    for (size_t i = 0; i < count; ++i) {
        memory_Block[num + i].allocated = false;
        memory_Block[num + i].count = 0;
    }
    allocated = NULL;
}

/**
 * @brief Function for initialize memory manager
 * 
 * @param size Size of allocable memory field
 */
void memory_init(uint64_t size) {
    if (size > UINT_MAX) { kernel_panic("64bit not supported"); }
    if (size < MEMORY_MINIMUMSIZE) { kernel_panic("Not enough memory detected"); }
    memory_Space = (void*)((size_t)&kernel_start + kernel_size() + 1);
    // ! Use low size for save power while debug and tests
    memory_Size = 4*1024*1024;//(size_t)size;
    memory_Block = (memory_Block_t*)memory_BlockBase;
    memory_BlockCount = memory_Size / MEMORY_BLOCKSIZE;
    for (int i = 0; i < memory_BlockCount; ++i) {
        memory_Block[i].count = 0;
        memory_Block[i].allocated = false;
        memory_Block[i].entry = (void*)((void*)memory_Space + (i * MEMORY_BLOCKSIZE));
    }
    fill(memory_Space, 0, memory_Size);
}

/**
 * @brief Function for write a memory report
 * 
 * @return Writed memory report as string
 */
char* memory_report() {
    size_t
        using_blocks = 0,
        using_size = 0,
        empty_blocks = 0,
        empty_size = 0;
    for (size_t i = 0; i < memory_BlockCount; ++i) {
        if (memory_Block[i].allocated) {
            using_blocks++;
            using_size += MEMORY_BLOCKSIZE;
        } else {
            empty_blocks++;
            empty_size += MEMORY_BLOCKSIZE;
        }
    }
    fill(memory_ReportMessage, 0, (size_t)256);
    snprintf(
        memory_ReportMessage, (size_t)256,
        "%d blocks (%d byte) in use, %d blocks (%d byte) free",
        using_blocks, using_size, empty_blocks, empty_size
    );
    return memory_ReportMessage;
}

/**
 * @brief Function for get allocated memory block information
 * 
 * @param allocated Allocated memory block
 * 
 * @return Allocated memory block information
 */
memory_Block_t* memory_blkInfo(void* allocated) {
    if (
        allocated == NULL ||
        (size_t)allocated < (size_t)memory_Space ||
        (size_t)allocated >= (size_t)memory_Space + memory_Size
    ) { return NULL; }
    return &memory_Block[((size_t)allocated - (size_t)memory_Space) / MEMORY_BLOCKSIZE];
}