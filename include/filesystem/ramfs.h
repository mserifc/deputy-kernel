#pragma once

#include "types.h"
#include "kernel.h"
#include "common.h"

// Constants

#define RAMFS_ROOTAREA_SIZE (256 * 1024)    // Define root directory size

#define RAMFS_MAX_NAME_LENGTH 28            // Define max name length for files and directories
#define RAMFS_MAX_FILE_COUNT 128            // Define max file capacity for directories
#define RAMFS_MAX_DIR_COUNT 63              // Define max directory capacity for root directory

// File structure
struct ramfs_File {
    char name[RAMFS_MAX_NAME_LENGTH];   // File name
    char* data;                         // Data field pointer
};

// Directory structure
struct ramfs_Directory {
    char name[RAMFS_MAX_NAME_LENGTH];               // Directory name
    uint32_t file_count;                            // File count
    struct ramfs_File file[RAMFS_MAX_FILE_COUNT];   // File array
};

// Root directory structure
struct ramfs_Root {
    uint32_t dir_count;                                 // Directory count
    struct ramfs_Directory dir[RAMFS_MAX_DIR_COUNT];    // Directory array
};

// Function prototypes

int ramfs_init();       // Function for initialize RAM file system
int ramfs_disable();    // Function for disable RAM file system

// Function for get root directory
struct ramfs_Root* ramfs_getRoot();

struct ramfs_Directory* ramfs_getDir(char* dirname);                // Function for get specific directory
struct ramfs_File* ramfs_getFile(char* dirname, char* filename);    // Function for get specific file

int ramfs_getDirIndex(char* dirname);                       // Function for get specific directory index in root directory
int ramfs_getFileIndex(char* dirname, char* filename);      // Function for get specific file index in specific directory

struct ramfs_File* ramfs_readDir(char* dirname);    // Function for read directory
int ramfs_createDir(char* dirname);                 // Function for create a directory
int ramfs_removeDir(char* dirname);                 // Function for remove a directory

char* ramfs_readFile(char* dirname, char* filename);                            // Function for read a file
int ramfs_writeFile(char* dirname, char* filename, char* data, size_t size);    // Function for write a file
int ramfs_removeFile(char* dirname, char* filename);                            // Function for remove a file