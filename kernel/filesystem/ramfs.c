#include "ramfs.h"

// Root Directory
struct ramfs_Directory ramfs_root;

// Reserved File List String
char ramfs_FileList[2048];

// Function for initialize RAM File System
int ramfs_init() {
    for (int i = 0; i < ramfs_MAX_FILE_COUNT; ++i) {
        memset(ramfs_root.file[i].name, 0, ramfs_MAX_FILE_NAMELENGTH);
        memset(ramfs_root.file[i].data, 0, ramfs_MAX_FILE_SIZE);
    }
    ramfs_root.file_count = 0;
    return 0;
}

// Function for find and get a specific file
struct ramfs_File* ramfs_findFile(char* name) {
    if ( ramfs_root.file_count < 0 ) { kernel_panic("Exception: Directory cannot contain less than 0 files."); }
    for (int i = 0; i < ramfs_root.file_count; ++i) {
        if (strcmp(ramfs_root.file[i].name, name) == 0) {
            return &ramfs_root.file[i];
        }
    }
    return null;
}

// Function for find and get a specific file index
int ramfs_findIndex(char* name) {
    if ( ramfs_root.file_count < 0 ) { kernel_panic("Exception: Directory cannot contain less than 0 files."); }
    for (int i = 0; i < ramfs_root.file_count; ++i) {
        if (strcmp(ramfs_root.file[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Function for get directory
struct ramfs_Directory* ramfs_getDirectory() { return &ramfs_root; }

// Function for read specific file
char* ramfs_readFile(char* name) {
    if ( ramfs_root.file_count < 0 ) { kernel_panic("Exception: Directory cannot contain less than 0 files."); }
    if (ramfs_findIndex(name) == -1) { return null; }
    return ramfs_root.file[ramfs_findIndex(name)].data;
}

// Function for write file
int ramfs_writeFile(char* name, char* data) {
    if ( ramfs_root.file_count < 0 ) { kernel_panic("Exception: Directory cannot contain less than 0 files."); }
    if (ramfs_findFile(name) == 0) {
        if (ramfs_root.file_count < ramfs_MAX_FILE_COUNT) {
            memset(ramfs_root.file[ramfs_root.file_count].name, 0, ramfs_MAX_FILE_NAMELENGTH);
            strcpy(ramfs_root.file[ramfs_root.file_count].name, name);
            ramfs_root.file[ramfs_root.file_count].name[ramfs_MAX_FILE_NAMELENGTH - 1] = '\0';
            memset(ramfs_root.file[ramfs_root.file_count].data, 0, ramfs_MAX_FILE_SIZE);
            strcpy(ramfs_root.file[ramfs_root.file_count].data, data);
            ramfs_root.file[ramfs_root.file_count].data[ramfs_MAX_FILE_SIZE - 1] = '\0';
            ramfs_root.file_count++;
            return 0;
        } else { return -1; }
    } else {
        memset(ramfs_findFile(name)->data, 0, ramfs_MAX_FILE_SIZE);
        strcpy(ramfs_findFile(name)->data, data);
        return 0;
    }
}

// Function for remove a specific file
int ramfs_removeFile(char* name) {
    if ( ramfs_root.file_count < 0 ) { kernel_panic("Exception: Directory cannot contain less than 0 files."); }
    int index = ramfs_findIndex(name);
    if (index != -1) {
        for (int i = index + 1; i < ramfs_root.file_count; ++i) {
            memcpy(ramfs_root.file[i - 1].name, ramfs_root.file[i].name, ramfs_MAX_FILE_NAMELENGTH);
            memcpy(ramfs_root.file[i - 1].data, ramfs_root.file[i].data, ramfs_MAX_FILE_SIZE);
        }
        memset(ramfs_root.file[ramfs_root.file_count].name, 0, ramfs_MAX_FILE_NAMELENGTH);
        memset(ramfs_root.file[ramfs_root.file_count].data, 0, ramfs_MAX_FILE_SIZE);
        ramfs_root.file_count--;
        return 0;
    } else { return -1; }
}
