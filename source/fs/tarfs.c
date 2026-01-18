#include "fs/tarfs.h"

#include "kernel.h"

int tarfs_list(void* base, size_t size) {
    size_t ofs = 0;
    while (!ncompare(((void*)(size_t)base + ofs + 257), "ustar", 5) && ofs < size) {
        tarfs_Entry_t* ent = (tarfs_Entry_t*)((size_t)base + ofs);
        printf("%s, %d, %d\n", ent->name, atoi(&ent->type), ofs);
        ofs += (((utils_oct2bin((char*)&ent->size, sizeof(ent->size)) + 511) / 512) + 1) * 512;
    } return 0;
}

int tarfs_mount(char* path, void* base, size_t size) {
    if (path == NULL || base == NULL || size == 0) { return -1; }
    if (fs_stat(path) == NULL) { ERR("Mount path not found"); return -1; }
    if (ncompare(((void*)(size_t)base + 257), "ustar", 5))
        { ERR("Unknown archive format"); return -1; }
    size_t ofs = 0;
    char* pathbuf = (char*)malloc(FS_MAX_PATHLEN); if (pathbuf == NULL) { ERR("Out of memory"); return -1; }
    while (!ncompare(((void*)(size_t)base + ofs + 257), "ustar", 5) && ofs < size) {
        tarfs_Entry_t* ent = (tarfs_Entry_t*)((size_t)base + ofs);
        snprintf(pathbuf, FS_MAX_PATHLEN, "%s%s", path, ent->name);
        if (atoi(&ent->type) == TARFS_TYPE_DIRECTORY) {
            if (fs_createDir(pathbuf) != 0) { ERR("Unable to mount directory at %s", pathbuf); return -1; }
        } else {
            if (fs_writeFile(pathbuf, 0, NULL) != 0) { ERR("Unable to mount file at %s", pathbuf); return -1; }
            fs_Entry_t* fent = fs_stat(pathbuf);
            if (fent == NULL) { ERR("File didn't mounted: %s", pathbuf); return -1; }
            int slot = mountmgr_getSlot(ent->data);
            if (slot == -1) { ERR("Unable to mount data of tar:/%s", ent->name); return -1; }
            fent->ftype = FS_TYPE_MOUNTED; fent->mountslot = slot;
            fent->size = (size_t)utils_oct2bin((char*)&ent->size, sizeof(ent->size) - 1);
        } ofs += (((utils_oct2bin((char*)&ent->size, sizeof(ent->size) - 1) + 511) / 512) + 1) * 512;
    } return 0;
}