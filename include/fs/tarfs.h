#pragma once

#include "types.h"

#define TARFS_TYPE_FILE         0
#define TARFS_TYPE_HARDLINK     1
#define TARFS_TYPE_SYMLINK      2
#define TARFS_TYPE_CHARDEV      3
#define TARFS_TYPE_BLKDEV       4
#define TARFS_TYPE_DIRECTORY    5
#define TARFS_TYPE_FIFOPIPE     6

typedef struct {
    char name[100];      // Name of the file or directory
    char mode[8];        // File mode
    char uid[8];         // User ID
    char gid[8];         // Group ID
    char size[12];       // Size of the file
    char mtime[12];      // Modification time
    char checksum[8];    // Checksum
    char type;           // Type of file
    char linkname[100];  // Name of the linked file
    char magic[6];       // Magic value
    char version[2];     // Version
    char uname[32];      // User name
    char gname[32];      // Group name
    char devmajor[8];    // Major device number
    char devminor[8];    // Minor device number
    char prefix[155];    // Prefix
    char padding[12];    // Padding
    char data[];         // Array pointing to the data area following the header
} PACKED tarfs_Entry_t;

int tarfs_list(void* base, size_t size);
int tarfs_mount(char* path, void* base, size_t size);