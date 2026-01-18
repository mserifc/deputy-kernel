#include "kernel.h"

// Pointor of allocable memory space for Memory Manager (Available memory space)
void* memory_Space;

// Allocable memory block list
struct memory_Block memory_BlockList[memory_BLOCKCOUNT];

// Memory report structure
struct memory_Report_t memory_Report;

// Memory report message buffer
char memory_ReportMessage[memory_REPORTMSGLENGTH];

// Initialize Memory Manager
void memory_init() {
    memory_Space = (void*)kernel_getSize() + 0x100000 + 1;
    for (size_t i = 0; i < memory_BLOCKCOUNT; ++i) {
        memory_BlockList[i].number = i;
        memory_BlockList[i].allocated = false;
        memory_BlockList[i].entry = (void*)((void*)memory_Space + (i * memory_BLOCKSIZE));
    }
    memset(memory_Space, 0, memory_SIZE);
}

// Allocate a memory block
void* memory_allocate() {
    for (size_t i = 0; i < memory_BLOCKCOUNT; ++i) {
        if (memory_BlockList[i].allocated == false) {
            memory_BlockList[i].allocated = true;
            return memory_BlockList[i].entry;
        }
    }
}

// Free a specific allocated memory blcok
void memory_free(void* allocated) {
    if (allocated == null) { return; }
    size_t num = ((size_t)allocated - (size_t)memory_Space) / memory_BLOCKSIZE;
    memory_BlockList[num].allocated = false;
    memset(allocated, 0, (size_t)memory_BLOCKSIZE);
    allocated = null;
}

// Give a memory report
struct memory_Report_t memory_report() {
    size_t using_blocks = 0;
    size_t using_size = 0;
    size_t empty_blocks = 0;
    size_t empty_size = 0;
    for (size_t i = 0; i < memory_BLOCKCOUNT; ++i) {
        if (memory_BlockList[i].allocated) {
            using_blocks++;
            using_size += memory_BLOCKSIZE;
        } else {
            empty_blocks++;
            empty_blocks += memory_BLOCKSIZE;
        }
    }
    memory_Report.using_blocks = using_blocks;
    memory_Report.using_size = using_size;
    memory_Report.empty_blocks = empty_blocks;
    memory_Report.empty_size = empty_size;
    return memory_Report;
}

// Write a memory report
char* memory_reportMessage() {
    size_t using_blocks = 0;
    size_t using_size = 0;
    size_t empty_blocks = 0;
    size_t empty_size = 0;
    for (size_t i = 0; i < memory_BLOCKCOUNT; ++i) {
        if (memory_BlockList[i].allocated) {
            using_blocks++;
            using_size += memory_BLOCKSIZE;
        } else {
            empty_blocks++;
            empty_blocks += memory_BLOCKSIZE;
        }
    }
    memset(memory_ReportMessage, 0, (size_t)memory_REPORTMSGLENGTH);
    snprintf(
        memory_ReportMessage, (size_t)memory_REPORTMSGLENGTH,
        "%d blocks (%d byte) in use, %d blocks (%d byte) free",
        using_blocks, using_size, empty_blocks, empty_size
    );
    return memory_ReportMessage;
}