#include "kernel.h"

#define MOUNTMGR_MOUNTLIMIT 1024

bool mountmgr_InitLock = false;

void** mountmgr_MountV;

int mountmgr_getSlot(void* data) {
    if (!mountmgr_InitLock || data == NULL) { return -1; }
    for (int i = 0; i < MOUNTMGR_MOUNTLIMIT; ++i) {
        if (mountmgr_MountV[i] == NULL) {
            mountmgr_MountV[i] = data;
            return i;
        }
    } ERR("Mount limit exceeded"); return -1;
}

int mountmgr_freeSlot(int slot) {
    if (!mountmgr_InitLock || mountmgr_MountV[slot] == NULL ||
        slot < 0 || slot >= MOUNTMGR_MOUNTLIMIT) { return -1; }
    mountmgr_MountV[slot] = NULL; return 0;
}

void mountmgr_init() {
    if (mountmgr_InitLock) { return; } mountmgr_InitLock = true;
    mountmgr_MountV = (void**)calloc(MOUNTMGR_MOUNTLIMIT, sizeof(void*));
    if (mountmgr_MountV == NULL) { PANIC("Out of memory"); }
}