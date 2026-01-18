#include "drv/ramdisk.h"

// ! Warning: This driver development failed

#include "kernel.h"

int ramdisk_create(ramdisk_t* disk, size_t blimit, size_t bsize) {
    if (!disk || blimit == 0 || bsize == 0) { return -1; }
    disk->bsize     = bsize;
    disk->blimit    = blimit;
    disk->storage   = calloc(blimit, bsize);
    if (!disk->storage) { return -1; } return 0;
}

int ramdisk_read(ramdisk_t* disk, size_t lba, void* buf, size_t num) {
    if (!disk || !buf) { return -1; }
    if (lba + num > disk->blimit) { return -1; }
    size_t off = lba * disk->bsize;
    size_t len = num * disk->bsize;
    ncopy(buf, (uint8_t*)disk->storage + off, len);
    return 0;
}

int ramdisk_write(ramdisk_t* disk, size_t lba, const void* buf, size_t num) {
    if (!disk || !buf) { return -1; }
    if (lba + num > disk->blimit) { return -1; }
    size_t off = lba * disk->bsize;
    size_t len = num * disk->bsize;
    ncopy(disk->storage + off, buf, len);
    return 0;
}

int ramdisk_remove(ramdisk_t* disk) {
    if (!disk || !disk->storage) { return -1; }
    free(disk->storage);
    disk->storage = NULL;
    disk->bsize = 0;
    disk->blimit = 0;
    return 0;
}