#pragma once

// ! Warning: This driver development failed

#include "types.h"

typedef struct {
    void* storage;
    size_t blimit;
    size_t bsize;
} ramdisk_t;

int ramdisk_create(ramdisk_t* disk, size_t blimit, size_t bsize);
int ramdisk_read(ramdisk_t* disk, size_t lba, void* buffer, size_t count);
int ramdisk_write(ramdisk_t* disk, size_t lba, const void* buffer, size_t count);
int ramdisk_remove(ramdisk_t* disk);