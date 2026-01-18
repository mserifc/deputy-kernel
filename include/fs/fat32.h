#pragma once

// ! This file system reader development incomplete

#include "types.h"
#include "drv/ramdisk.h"

extern ramdisk_t fat32_Disk;

int fat32_mount(uint32_t lba);