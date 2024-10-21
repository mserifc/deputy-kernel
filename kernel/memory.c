#include "kernel.h"

#define MemoryEntry 0x100000
#define MemorySize (1 * 1024 * 1024)
#define MemoryPageSize (16 * 1024)
#define MemoryPageCount (MemorySize / MemoryPageSize)

void* MemorySpace = (void*)MemoryEntry;

char MemoryUsageReport[100];

typedef struct {
    size_t number;
    bool allocted;
    void* entry;
} MemoryPage;

MemoryPage MemoryPageList[MemoryPageCount];

void memory_init() {
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        MemoryPageList[i].number = i;
        MemoryPageList[i].allocted = false;
        MemoryPageList[i].entry = (void*)(MemoryEntry + (i * MemoryPageSize));
    }
    memset(MemorySpace, 0, MemorySize);
}

void* memory_allocate() {
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        if (MemoryPageList[i].allocted == false) {
            MemoryPageList[i].allocted = true;
            return MemoryPageList[i].entry;
        }
    }
}

void memory_free(void* allocated) {
    if (allocated == null) { return; }
    size_t targetpagenumber = (((size_t)allocated) - MemoryEntry) / MemoryPageSize;
    MemoryPageList[targetpagenumber].allocted = false;
    memset(allocated, 0, (size_t)MemoryPageSize);
    allocated = null;
}

char* memory_report() {
    size_t usingblocks = 0;
    size_t usingsize = 0;
    size_t emptyblocks = 0;
    size_t emptysize = 0;
    for (size_t i = 0; i < MemoryPageCount; ++i) {
        if (MemoryPageList[i].allocted) {
            usingblocks++;
            usingsize+=MemoryPageSize;
        } else {
            emptyblocks++;
            emptysize+=MemoryPageSize;
        }
    }
    snprintf(MemoryUsageReport, 100,
        "%d blocks (%d byte) using, %d blocks (%d byte) free",
        usingblocks, usingsize, emptyblocks, emptysize);
    return MemoryUsageReport;
}
