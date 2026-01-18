#pragma once

#include "types.h"
#include "kernel.h"
#include "utils.h"

// * Constants

#define RAMFS_MAX_USERNAME_LENGTH   31          // Maximum username length
#define RAMFS_MAX_PATH_LENGTH       3839        // Maximum path length
#define RAMFS_MAX_NAME_LENGTH       255         // Maximum entry name length
#define RAMFS_MAX_ENTRY_COUNT       16384       // Maximum entry count

#define RAMFS_TYPE_FILE             0           // File type in file system
#define RAMFS_TYPE_HARDLINK         1           // Hard link type in file system
#define RAMFS_TYPE_SYMLINK          2           // Symbolic link type in file system
#define RAMFS_TYPE_CHARDEV          3           // Character device type in file system
#define RAMFS_TYPE_BLKDEV           4           // Block device type in file system
#define RAMFS_TYPE_DIR              5           // Directory type in file system
#define RAMFS_TYPE_FIFO             6           // Named pipe (FIFO) type in file system

#define RAMFS_ROOTDIR               0           // Index of root directory

#define RAMFS_DIRENTEND             -1          // End of directory entries

#define RAMFS_STATUS_SUCCESS        0           // File system operation success
#define RAMFS_STATUS_FAILURE        -1          // File system operation failure
#define RAMFS_STATUS_NOTINIT        -2          // File system not initialized
#define RAMFS_STATUS_ALREADYEXISTS  -3          // File/directory already exists
#define RAMFS_STATUS_OUTOFMEMORY    -4          // Out of memory
#define RAMFS_STATUS_PATHNOTFOUND   -5          // Path not found
#define RAMFS_STATUS_NOTDIR         -6          // Not an directory
#define RAMFS_STATUS_NOTFILE        -7          // Not an file
#define RAMFS_STATUS_ENTRYNOTFOUND  -8          // File/directory not found
#define RAMFS_STATUS_DIRNOTEMPTY    -9          // Directory not empty
#define RAMFS_STATUS_FILEEXISTS     -10         // File name exists
#define RAMFS_STATUS_DIREXISTS      -11         // Directory name exists

// * Structures

// Structure of RAM file system entry
typedef struct {
    char name[RAMFS_MAX_PATH_LENGTH + 1];           // Name of entry
    char user[RAMFS_MAX_USERNAME_LENGTH + 1];       // Owner username
    char group[RAMFS_MAX_USERNAME_LENGTH + 1];      // Owner group
    size_t size;                                    // Size of entry
    date_t ctime;                                   // Create time
    date_t mtime;                                   // Modify time
    date_t atime;                                   // Access time
    uint16_t perm;                                  // Permissions
    uint8_t type;                                   // Type of entry
} ramfs_Entry_t;

// * Public functions

int             ramfs_init();                                               // Initialize RAM file system

ramfs_Entry_t*  ramfs_stat(char* path);                                     // Get specific entry stats
ramfs_Entry_t*  ramfs_dirent(int entry);                                    // Get directory entries

int*            ramfs_readDir(char* path);                                  // Read specific directory
int             ramfs_createDir(char* path);                                // Create a directory

char*           ramfs_readFile(char* path);                                 // Read specific file
int             ramfs_writeFile(char* path, size_t size, char* buffer);     // Write a file

int             ramfs_remove(char* path);                                   // Remove a file/directory