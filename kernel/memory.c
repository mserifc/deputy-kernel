#include "kernel.h"

// Pointor of allocable memory space for Memory Manager (Available memory space)
void* memory_Space;

// Allocable memory block list
struct memory_Block memory_Blocklist[MEMORY_BLOCKCOUNT];

// Memory report message buffer
char memory_ReportMessage[MEMORY_REPORTMESSAGELENGTH];

// Function for initialize memory manager
void memory_init() {
    memory_Space = (void*)(KERNEL_ENTRY + kernel_getSize() + 1);
    for (size_t i = 0; i < MEMORY_BLOCKCOUNT; ++i) {
        memory_Blocklist[i].number = i;
        memory_Blocklist[i].count = 0;
        memory_Blocklist[i].allocated = false;
        memory_Blocklist[i].entry = (void*)((void*)memory_Space + (i * MEMORY_BLOCKSIZE));
    }
    memset(memory_Space, 0, MEMORY_SIZE);
}

// Function for allocate memory
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
    for (size_t i = 0; i < MEMORY_BLOCKCOUNT; ++i) {
        if (!memory_Blocklist[i].allocated) {
            for (size_t j = 0; j < count; ++j) {
                if (i + j >= MEMORY_BLOCKCOUNT) { return NULL; }
                if (memory_Blocklist[i + j].allocated) {
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
        memory_Blocklist[base + i].allocated = true;
    }
    memory_Blocklist[base].count = count;
    return memory_Blocklist[base].entry;
}

// Function for free a specific allocated memory blcok
void free(void* allocated) {
    if (
        allocated == NULL ||
        (size_t)allocated < (size_t)memory_Space ||
        (size_t)allocated >= (size_t)memory_Space + MEMORY_SIZE
    ) { return; }
    size_t num = ((size_t)allocated - (size_t)memory_Space) / MEMORY_BLOCKSIZE;
    size_t count = memory_Blocklist[num].count;
    for (size_t i = 0; i < count; ++i) {
        memory_Blocklist[num + i].allocated = false;
        memory_Blocklist[num + i].count = 0;
    }
    allocated = NULL;
}

// Function for get allocated memory block information
struct memory_Block* memory_blkInfo(void* allocated) {
    if (
        allocated == NULL ||
        (size_t)allocated < (size_t)memory_Space ||
        (size_t)allocated >= (size_t)memory_Space + MEMORY_SIZE
    ) { return NULL; }
    return &memory_Blocklist[((size_t)allocated - (size_t)memory_Space) / MEMORY_BLOCKSIZE];
}

// Function for write a memory report
char* memory_report() {
    size_t
        using_blocks = 0,
        using_size = 0,
        empty_blocks = 0,
        empty_size = 0;
    for (size_t i = 0; i < MEMORY_BLOCKCOUNT; ++i) {
        if (memory_Blocklist[i].allocated) {
            using_blocks++;
            using_size += MEMORY_BLOCKSIZE;
        } else {
            empty_blocks++;
            empty_size += MEMORY_BLOCKSIZE;
        }
    }
    memset(memory_ReportMessage, 0, (size_t)MEMORY_REPORTMESSAGELENGTH);
    snprintf(
        memory_ReportMessage, (size_t)MEMORY_REPORTMESSAGELENGTH,
        "%d blocks (%d byte) in use, %d blocks (%d byte) free",
        using_blocks, using_size, empty_blocks, empty_size
    );
    return memory_ReportMessage;
}