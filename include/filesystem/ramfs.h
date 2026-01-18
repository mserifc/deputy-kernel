#pragma once

#include "types.h"
#include "common.h"

// Define constants
#define ramfs_MAX_FILE_COUNT 64         // Max file count
#define ramfs_MAX_FILE_NAMELENGTH 32    // Max file name length
#define ramfs_MAX_FILE_SIZE 4 * 1024    // Max file size (4KB)

// Structure of file
struct ramfs_File {
    char name[ramfs_MAX_FILE_NAMELENGTH];   // File name
    char data[ramfs_MAX_FILE_SIZE];         // File data
};

// Structure of directory
struct ramfs_Directory {
    struct ramfs_File file[ramfs_MAX_FILE_COUNT];   // Contained files
    int file_count;                                 // File Count
};

// Function prototypes
int ramfs_init();                               // Initialize RAM File System
struct ramfs_File* ramfs_findFile(char* name);  // Find file
int ramfs_findIndex(char* name);                // Find file index
struct ramfs_Directory* ramfs_getDirectory();   // Get directory
char* ramfs_readFile(char* name);               // Read file
int ramfs_writeFile(char* name, char* data);    // Write file
int ramfs_removeFile(char* name);               // Remove file