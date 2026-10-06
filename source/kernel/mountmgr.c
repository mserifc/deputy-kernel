#include "kernel.h"

// * Constants

#define MOUNTMGR_MOUNTLIMIT 1024    // Mount limit

// * Variables and tables

// Initialize lock for prevent re-initializing mount manager
bool mountmgr_InitLock = false;

// Mount slots table
void** mountmgr_MountV;

/**
 * @brief Function for mount a data and get slot
 * 
 * @param data Data pointer
 * 
 * @return Mounted slot number (-1 mean failed)
 */
int mountmgr_getSlot(void* data) {
    if (!mountmgr_InitLock || data == NULL) { return -1; }
    for (int i = 0; i < MOUNTMGR_MOUNTLIMIT; ++i) {
        if (mountmgr_MountV[i] == NULL) {
            mountmgr_MountV[i] = data;
            return i;
        }
    } ERR("Mount limit exceeded"); return -1;
}

/**
 * @brief Function for release a mount slot
 * 
 * @param slot Slot number
 * 
 * @return Operation result (-1 mean failed)
 */
int mountmgr_freeSlot(int slot) {
    if (!mountmgr_InitLock ||
        slot < 0 || slot >= MOUNTMGR_MOUNTLIMIT ||
        mountmgr_MountV[slot] == NULL) { return -1; }
    mountmgr_MountV[slot] = NULL; return 0;
}

/**
 * @brief Function for initialize mount manager
 */
void mountmgr_init() {
    if (mountmgr_InitLock) { return; } mountmgr_InitLock = true;
    mountmgr_MountV = (void**)calloc(MOUNTMGR_MOUNTLIMIT, sizeof(void*));
    if (mountmgr_MountV == NULL) { PANIC("Out of memory"); }
}
