#include "filesystem/ramfs.h"

// Root area pointer
void* ramfs_RootArea;

// Root structure pointer
struct ramfs_Root* ramfs_root;

// RAM file system running status
bool ramfs_Running = false;

// Function for initialize RAM file system
int ramfs_init() {
    if (ramfs_Running) { return -1; }
    ramfs_RootArea = (void*)malloc(RAMFS_ROOTAREA_SIZE);
    if (ramfs_RootArea == NULL) { return -1; }
    ramfs_root = (struct ramfs_Root*)ramfs_RootArea;
    memset(ramfs_RootArea, 0, RAMFS_ROOTAREA_SIZE);
    ramfs_Running = true;
    return 0;
}

// Function for disable RAM file system
int ramfs_disable() {
    if (!ramfs_Running) { return -1; }
    free(ramfs_RootArea);
    ramfs_Running = false;
    return 0;
}

// Function for get root directory
struct ramfs_Root* ramfs_getRoot() {
    if (!ramfs_Running) { return NULL; }
    return ramfs_root;
}

// Function for get specific directory
struct ramfs_Directory* ramfs_getDir(char* dirname) {
    if (!ramfs_Running) { return NULL; }
    for (int i = 0; i < ramfs_root->dir_count; ++i) {
        if (strcmp(ramfs_root->dir[i].name, dirname) == 0) {
            return &ramfs_root->dir[i];
        }
    }
    return NULL;
}

// Function for get specific file
struct ramfs_File* ramfs_getFile(char* dirname, char* filename) {
    if (!ramfs_Running) { return NULL; }
    struct ramfs_Directory* dir = ramfs_getDir(dirname);
    if (dir == NULL) { return NULL; }
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->file[i].name, filename) == 0) {
            return &dir->file[i];
        }
    }
    return NULL;
}

// Function for get specific directory index in root directory
int ramfs_getDirIndex(char* dirname) {
    if (!ramfs_Running) { return -1; }
    for (int i = 0; i < ramfs_root->dir_count; ++i) {
        if (strcmp(ramfs_root->dir[i].name, dirname) == 0) {
            return i;
        }
    }
    return -1;
}

// Function for get specific file index in specific directory
int ramfs_getFileIndex(char* dirname, char* filename) {
    if (!ramfs_Running) { return -1; }
    struct ramfs_Directory* dir = ramfs_getDir(dirname);
    if (dir == NULL) { return -1; }
    for (int i = 0; i < dir->file_count; ++i) {
        if (strcmp(dir->file[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

// Function for read a directory
struct ramfs_File* ramfs_readDir(char* dirname) {
    if (!ramfs_Running) { return NULL; }
    struct ramfs_Directory* dir = ramfs_getDir(dirname);
    if (dir == NULL) { return NULL; }
    return dir->file;
}

// Function for create a directory
int ramfs_createDir(char* dirname) {
    if (!ramfs_Running) { return -1; }
    if (ramfs_getDirIndex(dirname) != -1) { return -1; }
    if (ramfs_root->dir_count < RAMFS_MAX_DIR_COUNT) {
        memset(ramfs_root->dir[ramfs_root->dir_count].name, 0, RAMFS_MAX_NAME_LENGTH);
        strncpy(ramfs_root->dir[ramfs_root->dir_count].name, dirname, RAMFS_MAX_NAME_LENGTH);
        ramfs_root->dir[ramfs_root->dir_count].name[RAMFS_MAX_NAME_LENGTH - 1] = '\0';
        memset((char*)ramfs_root->dir[ramfs_root->dir_count].file, 0, (RAMFS_MAX_FILE_COUNT * sizeof(struct ramfs_File)));
        ramfs_root->dir[ramfs_root->dir_count].file_count = 0;
        ramfs_root->dir_count++;
        return 0;
    } else { return -1; }
}

// Function for remove a directory
int ramfs_removeDir(char* dirname) {
    if (!ramfs_Running) { return -1; }
    uint32_t index = ramfs_getDirIndex(dirname);
    if (index == -1) { return -1; }
    if (ramfs_root->dir_count > 0) {
        for (int i = index + 1; i < ramfs_root->dir_count; ++i) {
            memcpy (&ramfs_root->dir[i - 1], &ramfs_root->dir[i], sizeof(struct ramfs_Directory));
        }
        memset(&ramfs_root->dir[ramfs_root->dir_count], 0, sizeof(struct ramfs_Directory));
        ramfs_root->dir_count--;
        return 0;
    } else { return -1; }
}

// Function for read a file
char* ramfs_readFile(char* dirname, char* filename) {
    if (!ramfs_Running) { return NULL; }
    int dir = ramfs_getDirIndex(dirname);
    if (dir == -1) { return NULL; }
    struct ramfs_File* file = ramfs_getFile(dirname, filename);
    if (file == NULL) { return NULL; }
    return file->data;
}

// Function for write a file
int ramfs_writeFile(char* dirname, char* filename, char* data, size_t size) {
    struct ramfs_Directory* dir = ramfs_getDir(dirname);
    if (dir == NULL) { ramfs_createDir(dirname); }
    dir = ramfs_getDir(dirname);
    if (dir == NULL) { return -1; }
    struct ramfs_File* file = ramfs_getFile(dirname, filename);
    if (file == NULL) {
        if (ramfs_root->dir_count < (RAMFS_MAX_FILE_COUNT - 1)) {
            void* allocdata = malloc(size);
            if (allocdata == NULL) { return -1; }
            memset(dir->file[dir->file_count].name, 0, RAMFS_MAX_NAME_LENGTH);
            strncpy(dir->file[dir->file_count].name, filename, RAMFS_MAX_NAME_LENGTH);
            dir->file[dir->file_count].name[RAMFS_MAX_NAME_LENGTH - 1] = '\0';
            dir->file[dir->file_count].data = allocdata;
            memcpy(dir->file[dir->file_count].data, data, size);
            dir->file_count++;
            return 0;
        } else { return -1; }
    } else {
        if (file->data == NULL) {
            void* data = malloc(size);
            if (data == NULL) { return -1; }
            file->data = data;
            memcpy(file->data, data, size);
            return 0;
        }
        struct memory_Block* file_data = memory_blkInfo(file->data);
        int count = (size + MEMORY_BLOCKSIZE - 1) / MEMORY_BLOCKSIZE;
        if (file_data->count != count) {
            void* data = malloc(size);
            if (data == NULL) { return -1; }
            free(file->data);
            file->data = data;
        }
        memcpy(file->data, data, size);
        return 0;
    }
}

// Function for remove a file
int ramfs_removeFile(char* dirname, char* filename) {
    if (!ramfs_Running) { return -1; }
    struct ramfs_Directory* dir = ramfs_getDir(dirname);
    if (dir == NULL) { return -1; }
    int index = ramfs_getFileIndex(dirname, filename);
    if (index != -1) {
        free(dir->file[index].data);
        for (int i = index + 1; i < dir->file_count; ++i) {
            memcpy(&dir->file[i - 1], &dir->file[i], sizeof(struct ramfs_File));
        }
        memset(&dir->file[dir->file_count], 0, sizeof(struct ramfs_File));
        dir->file_count--;
        return 0;
    } else { return -1; }
}