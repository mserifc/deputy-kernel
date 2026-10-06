#include "kernel.h"

// Initialize lock for prevent re-initializing userland
bool userland_InitLock = false;

void* userland_MemoryBase;  // Userland memory base
void* userland_MemoryLimit; // Userland memory limit

memory_Block_t* userland_MemBlockV; // Userland memory block table
size_t userland_MemBlockC;          // Userland allocable memory block count

/**
 * @brief Function for map a memory or file data
 * 
 * @param size Size of mapped memory or data
 * @param fd File descriptor (set -1 for map memory)
 * 
 * @return Mapped memory or data pointer
 */
void* mapmem(size_t size, int fd) {
    if (!userland_InitLock || size == 0)
        { return NULL; }

    if (fd == -1) {
        size_t count = (size + MEMORY_BLKSIZE - 1) / MEMORY_BLKSIZE;

        size_t base = 0;
        bool status = false;

        // Find free region
        if (count > userland_MemBlockC) { return NULL; }
        for (size_t i = 0; i <= userland_MemBlockC - count; i++) {
            bool ok = true;
            for (size_t j = 0; j < count; j++) {
                if (userland_MemBlockV[i + j].allocated) {
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
            userland_MemBlockV[base + i].allocated = true;
            userland_MemBlockV[base + i].count = 0;  // clear old info
        }
        userland_MemBlockV[base].count = count;

        return (void*)((size_t)userland_MemoryBase + (base * MEMORY_BLKSIZE));
    }
    fs_Entry_t* fent = iocall_getFileEntry(fd);
    if (fent == NULL) { return NULL; }
    return (void*)fs_readFile(fent->name);
}

/**
 * @brief Function for remap a mapped memory
 * 
 * WARNING: not available for userland
 * 
 * @param ptr Pointer of mapped memory
 * @param size Remap size
 * 
 * @return New mapped memory pointer
 */
void* remapmem(void* ptr, size_t size) {
    if (!userland_InitLock || ptr == NULL ||
        (size_t)ptr < (size_t)userland_MemoryBase ||
        (size_t)ptr >= (size_t)userland_MemoryLimit
    ) { return NULL; }
    size_t num = ((size_t)ptr - (size_t)userland_MemoryBase) / MEMORY_BLKSIZE;
    if (!userland_MemBlockV[num].allocated) { return NULL; }
    size_t count = userland_MemBlockV[num].count;
    if (count == 0) { return NULL; }  // invalid free
    void* newblk = mapmem(size, -1);
    if (newblk == NULL) { return NULL; }
    size_t oldsize = count * MEMORY_BLKSIZE;
    size_t copysize = oldsize < size ? oldsize : size;
    ncopy(newblk, ptr, copysize);
    unmapmem(ptr);
    return newblk;
}

/**
 * @brief Function for unmap a mapped memory or file data
 * 
 * @param ptr Mapped memory pointer to unmap
 */
void unmapmem(void* ptr) {
    if (!userland_InitLock || ptr == NULL ||
        (size_t)ptr < (size_t)userland_MemoryBase ||
        (size_t)ptr >= (size_t)userland_MemoryLimit
    ) { return; }

    size_t num = ((size_t)ptr - (size_t)userland_MemoryBase) / MEMORY_BLKSIZE;
    if (!userland_MemBlockV[num].allocated) { return; }
    size_t count = userland_MemBlockV[num].count;

    if (count == 0) { return; }  // invalid free

    for (size_t i = 0; i < count; i++) {
        userland_MemBlockV[num + i].allocated = false;
        userland_MemBlockV[num + i].count = 0;
    }
}

/**
 * @brief Function for get available memory of userland memory
 * 
 * @return Free memory size
 */
size_t userland_availmem() {
    if (!userland_InitLock) { return 0; }
    size_t result = 0;
    for (size_t i = 0; i < userland_MemBlockC; i++) {
        if (!userland_MemBlockV[i].allocated) { result += MEMORY_BLKSIZE; }
    } return result;
}

/**
 * @brief Function for initialize userland
 * 
 * @param base Possible base address of userland
 * @param limit Limit address of userland
 * 
 * @return New base address after calculation
 */
size_t userland_init(size_t base, size_t limit) {
    if (userland_InitLock) { return -1; }

    size_t supblkc = (limit - base) / (MEMORY_BLKSIZE + sizeof(memory_Block_t));
    supblkc -= supblkc ? 1 : 0;
    if (supblkc == 0) { PANIC("Not enough memory detected"); }

    userland_MemoryBase = (void*)(base + (supblkc * sizeof(memory_Block_t)));
    userland_MemoryBase = (void*)(((size_t)userland_MemoryBase + MEMORY_BLKSIZE) & ~((size_t)MEMORY_BLKSIZE - 1));
    userland_MemoryLimit = (void*)((size_t)userland_MemoryBase + (supblkc * MEMORY_BLKSIZE));

    userland_MemBlockV = (memory_Block_t*)(base);
    userland_MemBlockC = supblkc;

    for (size_t i = 0; i < userland_MemBlockC; i++) {
        userland_MemBlockV[i].count = 0;
        userland_MemBlockV[i].allocated = false;
    } userland_InitLock = true;
    return (size_t)userland_MemoryBase;
}