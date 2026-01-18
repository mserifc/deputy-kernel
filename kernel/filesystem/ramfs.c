#include "filesystem/ramfs.h"

// Initialize state of RAM file system
bool ramfs_Running = false;

// Entries of RAM file system
void* ramfs_Entry[RAMFS_MAX_ENTRY_COUNT];

// Buffer for directory entries
int ramfs_DirentBuffer[RAMFS_MAX_ENTRY_COUNT];

// Buffer for entry paths
char ramfs_PathBuffer[RAMFS_MAX_PATH_LENGTH + 1];

/**
 * @brief Function for initialize RAM file system
 * 
 * @return Status code
 */
int ramfs_init() {
    if (ramfs_Running) { return RAMFS_STATUS_FAILURE; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) { ramfs_Entry[i] = NULL; }
    ramfs_Entry[RAMFS_ROOTDIR] = malloc(sizeof(ramfs_Entry_t));
    if (ramfs_Entry[RAMFS_ROOTDIR] == NULL)
        { kernel_panic("Unable to initialize RAM file system: Out of memory"); }
    ramfs_Entry_t* rootdir = (ramfs_Entry_t*)ramfs_Entry[RAMFS_ROOTDIR];
    copy(rootdir->name, "/");
    copy(rootdir->user, "deputy");
    copy(rootdir->group, "deputy");
    rootdir->ctime = date();
    rootdir->mtime = date();
    rootdir->atime = date();
    rootdir->perm = 0777;
    rootdir->type = RAMFS_TYPE_DIR;
    ramfs_Running = true; return RAMFS_STATUS_SUCCESS;
}

/**
 * @brief Function for get stats of specific entry
 * 
 * @param path Path of specific entry
 * 
 * @return Information table of entry
 */
ramfs_Entry_t* ramfs_stat(char* path) {
    if (!ramfs_Running) { return NULL; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ent->name[0] != '\0' && compare(ent->name, path) == 0) { return ent; }
    } return NULL;
}

/**
 * @brief Function for get specific entry from directory
 * 
 * @param entry Specific entry from directory
 * 
 * @return Information table of entry
 */
ramfs_Entry_t* ramfs_dirent(int entry) {
    if (!ramfs_Running) { return NULL; }
    return (ramfs_Entry_t*)ramfs_Entry[entry];
}

/**
 * @brief Function for read directory entries
 * 
 * @param path Path of specific directory
 * 
 * @return Array of entry indexes
 */
int* ramfs_readDir(char* path) {
    if (!ramfs_Running) { return NULL; }
    if (length(path) == 0 || path[length(path) - 1] != '/') { return NULL; }
    if (path && ncompare(path, "/", length(path)) == 0) {
        for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) { ramfs_DirentBuffer[i] = RAMFS_DIRENTEND; }
        int j = 0; for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
            if (ramfs_Entry[i] == NULL) { continue; }
            ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
            if (ent->name && ncompare(ent->name, path, length(path)) == 0)
                { ramfs_DirentBuffer[j] = i; ++j; }
        } return ramfs_DirentBuffer;
    }
    bool found = false;
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ent->name[0] != '\0' && compare(ent->name, path) == 0)
            { if (ent->type == RAMFS_TYPE_DIR) { found = true; } break; }
    } if (!found) { return NULL; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) { ramfs_DirentBuffer[i] = RAMFS_DIRENTEND; }
    int j = 0; for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ncompare(ent->name, path, length(path)) == 0)
            { ramfs_DirentBuffer[j] = i; ++j; }
    }
    ramfs_stat(path)->atime = date();
    return ramfs_DirentBuffer;
}

/**
 * @brief Function for create a directory
 * 
 * @param path Path of new directory
 * 
 * @return Status code
 */
int ramfs_createDir(char* path) {
    if (!ramfs_Running) { return RAMFS_STATUS_NOTINIT; }
    if (length(path) == 0 || path[length(path) - 1] != '/') { return RAMFS_STATUS_FAILURE; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ncompare(ent->name, path, length(path) - 1) == 0)
            { if (ent->type == RAMFS_TYPE_FILE) { return RAMFS_STATUS_FILEEXISTS; } }
    }
    bool found = false;
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ent->name[0] != '\0' && compare(ent->name, path) == 0)
            { if (ent->type == RAMFS_TYPE_DIR) { found = true; } break; }
    } if (found) { return RAMFS_STATUS_ALREADYEXISTS; }
    {
        tokens_t dirs = split(path, '/');
        fill(ramfs_PathBuffer, 0, RAMFS_MAX_PATH_LENGTH + 1);
        ramfs_PathBuffer[0] = '/';
        int bufptr = 1;
        for (int i = 0; i < dirs.c - 1; ++i) {
            for (int j = 0; j <= i; ++j) {
                copy(&ramfs_PathBuffer[bufptr], dirs.v[j]);
                bufptr += length(dirs.v[j]);
                ramfs_PathBuffer[bufptr] = '/';
                bufptr++;
            }
            bool parent_found = false;
            for (int j = 0; j < RAMFS_MAX_ENTRY_COUNT; ++j) {
                if (ramfs_Entry[j] == NULL) { continue; }
                ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[j];
                if (ent->name && compare(ent->name, ramfs_PathBuffer) == 0 && ent->type == RAMFS_TYPE_DIR) {
                    parent_found = true;
                    break;
                }
            } if (!parent_found) { return RAMFS_STATUS_PATHNOTFOUND; }
            bufptr = 1;
            fill(ramfs_PathBuffer, 0, RAMFS_MAX_PATH_LENGTH + 1);
            ramfs_PathBuffer[0] = '/';
        }
    }
    int index = 0;
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { index = i; break; }
    } if (index <= 0) { return RAMFS_STATUS_OUTOFMEMORY; }
    void* newptr = malloc(MEMORY_BLOCKSIZE);
    if (newptr == NULL) { return RAMFS_STATUS_OUTOFMEMORY; }
    ramfs_Entry[index] = newptr;
    ramfs_Entry_t* dir = (ramfs_Entry_t*)ramfs_Entry[index];
    copy(dir->name, path);
    copy(dir->user, "root");
    copy(dir->group, "root");
    dir->ctime = date();
    dir->mtime = date();
    dir->atime = date();
    dir->perm = 0777;
    dir->type = RAMFS_TYPE_DIR;
    return RAMFS_STATUS_SUCCESS;
}

/**
 * @brief Function for read a specific file content
 * 
 * @param path Path of specific file
 * 
 * @return Buffer of file content
 */
char* ramfs_readFile(char* path) {
    if (!ramfs_Running) { return NULL; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ent->name[0] != '\0' && compare(ent->name, path) == 0) {
            if (ent->type == RAMFS_TYPE_FILE) {
                ramfs_stat(path)->atime = date();
                return (char*)((char*)ramfs_Entry[i] + (MEMORY_BLOCKSIZE));
            } break;
        }
    } return NULL;
}

/**
 * @brief Function for write a file
 * 
 * @param path Path of file
 * @param size New size of file
 * @param buffer Buffer for write to file
 * 
 * @return Status code
 */
int ramfs_writeFile(char* path, size_t size, char* buffer) {
    if (!ramfs_Running) { return RAMFS_STATUS_NOTINIT; }
    if (length(path) == 0 || path[length(path) - 1] == '/') { return RAMFS_STATUS_FAILURE; }
    if (buffer == NULL) { return RAMFS_STATUS_FAILURE; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->type == RAMFS_TYPE_DIR && ent->name) {
            size_t ent_name_len = length(ent->name);
            if (ent_name_len > 0 && ent->name[ent_name_len - 1] == '/') {
                if (length(path) + 1 == ent_name_len)
                    { if (ncompare(ent->name, path, length(path)) == 0) { return RAMFS_STATUS_DIREXISTS; } }
            }
        }
    }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ent->name[0] != '\0' && compare(ent->name, path) == 0) {
            if (ent->type == RAMFS_TYPE_DIR) { return RAMFS_STATUS_NOTFILE; }
            void* newptr = realloc(ramfs_Entry[i], size + MEMORY_BLOCKSIZE);
            if (newptr == NULL) { return RAMFS_STATUS_OUTOFMEMORY; } ramfs_Entry[i] = newptr;
            ncopy((char*)ramfs_Entry[i] + MEMORY_BLOCKSIZE, buffer, size);
            ent = (ramfs_Entry_t*)ramfs_Entry[i];
            ent->size = size; ent->mtime = date(); ent->atime = date();
            return RAMFS_STATUS_SUCCESS;
        }
    }
    {
        tokens_t dirs = split(path, '/');
        fill(ramfs_PathBuffer, 0, RAMFS_MAX_PATH_LENGTH + 1);
        ramfs_PathBuffer[0] = '/';
        int bufptr = 1;
        for (int i = 0; i < dirs.c - 1; ++i) {
            for (int j = 0; j <= i; ++j) {
                copy(&ramfs_PathBuffer[bufptr], dirs.v[j]);
                bufptr += length(dirs.v[j]);
                ramfs_PathBuffer[bufptr] = '/';
                bufptr++;
            }
            bool parent_found = false;
            for (int j = 0; j < RAMFS_MAX_ENTRY_COUNT; ++j) {
                if (ramfs_Entry[j] == NULL) { continue; }
                ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[j];
                if (ent->name && compare(ent->name, ramfs_PathBuffer) == 0 && ent->type == RAMFS_TYPE_DIR) {
                    parent_found = true;
                    break;
                }
            } if (!parent_found) { return RAMFS_STATUS_PATHNOTFOUND; }
            bufptr = 1;
            fill(ramfs_PathBuffer, 0, RAMFS_MAX_PATH_LENGTH + 1);
            ramfs_PathBuffer[0] = '/';
        }
    }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) {
            void* newptr = malloc(MEMORY_BLOCKSIZE + size);
            if (newptr == NULL) { return RAMFS_STATUS_OUTOFMEMORY; }
            ramfs_Entry[i] = newptr;
            ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
            copy(ent->name, path);
            copy(ent->user, "root");
            copy(ent->group, "root");
            ent->size = size;
            ent->ctime = date();
            ent->mtime = date();
            ent->atime = date();
            ent->perm = 0777;
            ent->type = RAMFS_TYPE_FILE;
            ncopy((char*)ramfs_Entry[i] + MEMORY_BLOCKSIZE, buffer, size);
            return RAMFS_STATUS_SUCCESS;
        }
    } return RAMFS_STATUS_OUTOFMEMORY;
}

/**
 * @brief Function for remove a entry
 * 
 * @param path Path of specific entry
 * 
 * @return Status code
 */
int ramfs_remove(char* path) {
    if (!ramfs_Running) { return RAMFS_STATUS_NOTINIT; }
    if (compare(path, "/") == 0) { return RAMFS_STATUS_FAILURE; }
    for (int i = 0; i < RAMFS_MAX_ENTRY_COUNT; ++i) {
        if (ramfs_Entry[i] == NULL) { continue; }
        ramfs_Entry_t* ent = (ramfs_Entry_t*)ramfs_Entry[i];
        if (ent->name && ent->name[0] != '\0' && compare(ent->name, path) == 0) {
            if (ent->type == RAMFS_TYPE_DIR) {
                int pathlen = length(path);
                fill(ramfs_PathBuffer, 0, RAMFS_MAX_PATH_LENGTH + 1);
                copy(ramfs_PathBuffer, path);
                if (path[pathlen - 1] != '/') {
                    ramfs_PathBuffer[pathlen] = '/';
                    ramfs_PathBuffer[pathlen + 1] = '\0';
                    pathlen++;
                }
                for (int j = 0; j < RAMFS_MAX_ENTRY_COUNT; ++j) {
                    if (ramfs_Entry[j] == NULL) { continue; }
                    ramfs_Entry_t* ent2 = (ramfs_Entry_t*)ramfs_Entry[j];
                    if (
                        ent2->name &&
                        ncompare(ent2->name, ramfs_PathBuffer, pathlen) == 0 &&
                        length(ent2->name) > pathlen
                    ) { return RAMFS_STATUS_DIRNOTEMPTY; }
                }
            } free(ramfs_Entry[i]); ramfs_Entry[i] = NULL; return RAMFS_STATUS_SUCCESS;
        }
    } return RAMFS_STATUS_ENTRYNOTFOUND;
}