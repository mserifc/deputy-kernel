#include "filesystem/ownfs.h"

// Define file system memory field pointer and root directory
void* ownfs_WholeArea;              // Define memory field pointer
struct ownfs_Root* ownfs_root;      // Define root directory

// Calculate maximum directory count by whole size
uint32_t ownfs_MaxDirCount = (OWNFS_WHOLEAREA_SIZE - sizeof(uint64_t)) / sizeof(struct ownfs_Directory);

// Running status variable
bool ownfs_Running = false;

// Function for load file system from disk
int ownfs_load() {
    if (!ownfs_Running || !disk_support()) { return -1; }
    uint8_t* buffer = (uint8_t*)malloc(DISK_SECTOR_SIZE);
    if (disk_readSector(1, buffer) == -1) { free(buffer); return -1; }
    if (((struct ownfs_Root*)buffer)->signature == OWNFS_SIGNATURE) {
        for (int i = 0; i < (OWNFS_WHOLEAREA_SIZE / DISK_SECTOR_SIZE); ++i) {
            if (disk_readSector(i + 1, (uint8_t*)ownfs_WholeArea + (DISK_SECTOR_SIZE * i)) == -1) { return -1; }
        }
        free(buffer);
    } else { free(buffer); return -1; }
    return 0;
}

// Function for save file system to disk
int ownfs_save() {
    if (!ownfs_Running || !disk_support()) { return -1; }
    for (int i = 0; i < (OWNFS_WHOLEAREA_SIZE / DISK_SECTOR_SIZE); ++i) {
        if (disk_writeSector(i + 1, (uint8_t*)ownfs_WholeArea + (DISK_SECTOR_SIZE * i)) == -1) { return -1; }
    }
    return 0;
}

// Function for Initialize file system
int ownfs_init() {
    if (ownfs_Running) { return -1; }
    ownfs_WholeArea = (void*)malloc(OWNFS_WHOLEAREA_SIZE);
    if (ownfs_WholeArea == NULL) { return -1; }
    ownfs_root = (struct ownfs_Root*)ownfs_WholeArea;
    memset(ownfs_WholeArea, 0, OWNFS_WHOLEAREA_SIZE);
    ownfs_root->signature = OWNFS_SIGNATURE;
    ownfs_Running = true;
    return 0;
}

// Function for disable file system (free memory)
int ownfs_disable() {
    if (!ownfs_Running) { return -1; }
    free(ownfs_WholeArea);
    ownfs_Running = false;
    return 0;
}

// Function for get root directory
struct ownfs_Root* ownfs_getRoot() {
    if (!ownfs_Running) { return NULL; }
    return ownfs_root;
}

// Function for find specific directory by name
struct ownfs_Directory* ownfs_findDir(char* dirname) {
    if (!ownfs_Running) { return NULL; }
    for (int i = 0; i < ownfs_root->dir_count; ++i) {
        if (strcmp(ownfs_root->dir[i].name, dirname) == 0) {
            return &ownfs_root->dir[i];
        }
    }
    return NULL;
}

// Function for find specific file in directory by name
struct ownfs_File* ownfs_findFile(char* dirname, char* filename) {
    if (!ownfs_Running) { return NULL; }
    struct ownfs_Directory* dir = ownfs_findDir(dirname);
    if (dir == NULL) { return NULL; }
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->file[i].name, filename) == 0) {
            return &dir->file[i];
        }
    }
    return NULL;
}

// Function for find specific directory index by name
int ownfs_findDirIndex(char* dirname) {
    if (!ownfs_Running) { return -1; }
    for (int i = 0; i < ownfs_root->dir_count; ++i) {
        if (strcmp(ownfs_root->dir[i].name, dirname) == 0) {
            return i;
        }
    }
    return -1;
}

// Function for find specific file index in directory by name
int ownfs_findFileIndex(char* dirname, char* filename) {
    if (!ownfs_Running) { return -1; }
    struct ownfs_Directory* dir = ownfs_findDir(dirname);
    if (dir == NULL) { return -1; }
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->file[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Function for find specific directory and get contents
struct ownfs_File* ownfs_readDir(char* dirname) {
    if (!ownfs_Running) { return NULL; }
    struct ownfs_Directory* dir = ownfs_findDir(dirname);
    if (dir == NULL) { return NULL; }
    return dir->file;
}

// Function for create a directory
int ownfs_createDir(char* dirname) {
    if (!ownfs_Running) { return -1; }
    if (ownfs_findDir(dirname) != NULL) { return -1; }
    if (ownfs_root->dir_count < (ownfs_MaxDirCount - 1)) {
        memset(ownfs_root->dir[ownfs_root->dir_count].name, 0, OWNFS_MAX_NAME_LENGTH);
        strncpy(ownfs_root->dir[ownfs_root->dir_count].name, dirname, OWNFS_MAX_NAME_LENGTH);
        ownfs_root->dir[ownfs_root->dir_count].name[OWNFS_MAX_NAME_LENGTH - 1] = '\0';
        memset((char*)ownfs_root->dir[ownfs_root->dir_count].file, 0, (OWNFS_MAX_FILE_COUNT * sizeof(struct ownfs_File)));
        ownfs_root->dir[ownfs_root->dir_count].file_count = 0;
        ownfs_root->dir_count++;
        return 0;
    } else { return -1; }
}

// Function for remove a directory
int ownfs_removeDir(char* dirname) {
    if (!ownfs_Running) { return -1; }
    uint32_t index = ownfs_findDirIndex(dirname);
    if (index == -1) { return -1; }
    if (ownfs_root->dir_count > 0) {
        for (int i = index + 1; i < ownfs_root->dir_count; ++i) {
            memcpy(&ownfs_root->dir[i - 1], &ownfs_root->dir[i], sizeof(struct ownfs_Directory));
        }
        memset(&ownfs_root->dir[ownfs_root->dir_count], 0, sizeof(struct ownfs_Directory));
        ownfs_root->dir_count--;
        return 0;
    } else { return -1; }
}

// Function for read data from specific file
char* ownfs_readFile(char* dirname, char* filename) {
    if (!ownfs_Running) { return NULL; }
    struct ownfs_Directory* dir = ownfs_findDir(dirname);
    if (dir == NULL) { return NULL; }
    struct ownfs_File* file = ownfs_findFile(dirname, filename);
    if (file == NULL) { return NULL; }
    return file->data;
}

// Function for write a file
int ownfs_writeFile(char* dirname, char* filename, char* data) {
    if (!ownfs_Running) { return -1; }
    struct ownfs_Directory* dir = ownfs_findDir(dirname);
    if (dir == NULL) { ownfs_createDir(dirname); }
    dir = ownfs_findDir(dirname);
    if (dir == NULL) { return -1; }
    struct ownfs_File* file = ownfs_findFile(dirname, filename);
    if (file == NULL) {
        if (ownfs_root->dir_count < (OWNFS_MAX_FILE_COUNT - 1)) {
            memset(dir->file[dir->file_count].name, 0, OWNFS_MAX_NAME_LENGTH);
            strncpy(dir->file[dir->file_count].name, filename, OWNFS_MAX_NAME_LENGTH);
            dir->file[dir->file_count].name[OWNFS_MAX_NAME_LENGTH - 1] = '\0';
            memset(dir->file[dir->file_count].data, 0, OWNFS_MAX_DATA_SIZE);
            strncpy(dir->file[dir->file_count].data, data, OWNFS_MAX_DATA_SIZE);
            dir->file[dir->file_count].data[OWNFS_MAX_DATA_SIZE - 1] = '\0';
            dir->file_count++;
            return 0;
        } else { return -1; }
    } else {
        memset(file->data, 0, OWNFS_MAX_DATA_SIZE);
        strncpy(file->data, data, OWNFS_MAX_DATA_SIZE);
        return 0;
    }
}

// Function for remove a file
int ownfs_removeFile(char* dirname, char* filename) {
    if (!ownfs_Running) { return -1; }
    struct ownfs_Directory* dir = ownfs_findDir(dirname);
    if (dir == NULL) { return -1; }
    int index = ownfs_findFileIndex(dirname, filename);
    if (index != -1) {
        for (int i = index + 1; i < dir->file_count; ++i) {
            memcpy(&dir->file[i - 1], &dir->file[i], sizeof(struct ownfs_File));
        }
        memset(&dir->file[dir->file_count], 0, sizeof(struct ownfs_File));
        dir->file_count--;
        return 0;
    } else { return -1; }
}