#pragma once

#include "types.h"
#include "kernel.h"
#include "common.h"
#include "drivers/disk.h"

// Constants

#define OWNFS_WHOLEAREA_SIZE (4 * 1024 * 1024)  // Define whole file system area size

#define OWNFS_MAX_NAME_LENGTH 32                // Define max name length for files and directories
#define OWNFS_MAX_FILE_COUNT 64                 // Define max file count in directory
#define OWNFS_MAX_DATA_SIZE (4 * 1024)          // Define max size for file contents

#define OWNFS_SIGNATURE 0x534B4653              // Own kernel file system signature

// File structure
struct ownfs_File {
    char name[OWNFS_MAX_NAME_LENGTH];
    char data[OWNFS_MAX_DATA_SIZE];
};

// Directory structure
struct ownfs_Directory {
    char name[OWNFS_MAX_NAME_LENGTH];
    uint32_t file_count;
    struct ownfs_File file[OWNFS_MAX_FILE_COUNT];
};

// Root structure
struct ownfs_Root {
    uint32_t signature;
    uint32_t dir_count;
    struct ownfs_Directory dir[(OWNFS_WHOLEAREA_SIZE - sizeof(uint64_t)) / sizeof(struct ownfs_Directory)];
};

// Function prototypes

int ownfs_load();       // Load file system from disk
int ownfs_save();       // Save file system to disk

int ownfs_init();       // Initialize file system
int ownfs_disable();    // Disable file system (free memory)

struct ownfs_Root* ownfs_getRoot();     // Get root directory

struct ownfs_Directory* ownfs_findDir(char* dirname);               // Find specific directory by name
struct ownfs_File* ownfs_findFile(char* dirname, char* filename);   // Find specific file in directory by name

int ownfs_findDirIndex(char* dirname);                      // Find specific directory index by name
int ownfs_findFileIndex(char* dirname, char* filename);     // Find specific file index in directory by name

struct ownfs_File* ownfs_readDir(char* dirname);    // Find specific directory and get contents
int ownfs_createDir(char* dirname);                 // Create a directory
int ownfs_removeDir(char* dirname);                 // Remove a directory

char* ownfs_readFile(char* dirname, char* filename);                // Read data from specific file
int ownfs_writeFile(char* dirname, char* filename, char* data);     // Write a file
int ownfs_removeFile(char* dirname, char* filename);                // Remove a file