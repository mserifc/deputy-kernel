#include "kernel.h"

// Initialize lock for prevent re-initializing core file system
bool fs_InitLock = false;

// Entry vector in core file system
fs_Entry_t** fs_EntryV;

// Random access directory entry list
int* fs_RADirent;

// Random access path buffer 
char* fs_RAPath;

/**
 * @brief Function for get index of specific entry in entry table
 * 
 * @param path Path of specific entry
 * 
 * @return Index of specific entry
 */
int fs_index(const char* path) {
    if (!fs_InitLock) { return FS_STS_NOTINIT; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0) { return i; }
    } return FS_STS_ENTRYNOTFOUND;
}

/**
 * @brief Function for get stats of specific entry
 * 
 * @param path Path of specific entry
 * 
 * @return Information table of entry
 */
fs_Entry_t* fs_stat(const char* path) {
    if (!fs_InitLock || path == NULL) { return NULL; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0) { return ent; }
    } return NULL;
}

/**
 * @brief Function for get parent directory of specific entry
 * 
 * @param path Path of specific entry
 * 
 * @return Information table of parent entry
 */
fs_Entry_t* fs_parent(const char* path) {
    if (!fs_InitLock || compare(path, "/") == 0) { return NULL; }
    int end = 0; for (int i = 0; i < length(path) - 1; ++i) { if (path[i] == '/') { end = i + 1; } }
    fill(fs_RAPath, 0, FS_MAX_PATHLEN + 1); ncopy(fs_RAPath, path, end);
    return fs_stat(fs_RAPath);
}

/**
 * @brief Function for get specific entry from directory
 * 
 * @param entry Specific entry from directory
 * 
 * @return Information table of entry
 */
fs_Entry_t* fs_dirent(int entry) {
    if (!fs_InitLock) { return NULL; }
    return (fs_Entry_t*)fs_EntryV[entry];
}

/**
 * @brief Function for check operation permission of specific entry
 * 
 * @param ent Specific entry
 * @param uid User ID
 * @param gid Group ID
 * @param perm Access permission
 * 
 * @return Has access or not (true/false)
 */
bool fs_checkPerm(fs_Entry_t* ent, int uid, int gid, uint8_t perm) {
    if (!fs_InitLock) { return FS_STS_NOTINIT; }
    if (ent == NULL) { return false; }
    if (uid == ent->user) {
        if (((ent->perm >> 6) & 7) & perm) return true;
    } else if (gid == ent->group) {
        if (((ent->perm >> 3) & 7) & perm) return true;
    } else {
        if ((ent->perm & 7) & perm) return true;
    } return false;
}

/**
 * @brief Function for read directory entries
 * 
 * @param path Path of specific directory
 * 
 * @return Array of entry indexes
 */
int* fs_readDir(const char* path) {
    if (!fs_InitLock) { return NULL; }
    if (length(path) == 0 || path[length(path) - 1] != '/') { return NULL; }
    if (path && ncompare(path, "/", length(path)) == 0) {
        for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) { fs_RADirent[i] = FS_DIRENTEND; }
        int j = 0; for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
            if (fs_EntryV[i] == NULL) { continue; }
            fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
            if (ent->name[0] != '\0' && ncompare(ent->name, path, length(path)) == 0)
                { fs_RADirent[j] = i; ++j; }
        } return fs_RADirent;
    }
    bool found = false;
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0)
            { if (ent->type == FS_TYPE_DIR) { found = true; } break; }
    } if (!found) { return NULL; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) { fs_RADirent[i] = FS_DIRENTEND; }
    int j = 0; for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ncompare(ent->name, path, length(path)) == 0)
            { fs_RADirent[j] = i; ++j; }
    }
    date(&fs_stat(path)->atime);
    return fs_RADirent;
}

/**
 * @brief Function for create a directory
 * 
 * @param path Path of new directory
 * 
 * @return Status code
 */
int fs_createDir(const char* path) {
    if (!fs_InitLock) { return FS_STS_NOTINIT; }
    if (length(path) == 0 ||
        path[length(path) - 1] != '/' ||
        length(path) >= FS_MAX_PATHLEN) { return FS_STS_FAILURE; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ncompare(ent->name, path, length(path) - 1) == 0)
            { if (ent->type == FS_TYPE_FILE) { return FS_STS_FILEEXISTS; } }
    }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0)
            { if (ent->type == FS_TYPE_DIR) { return FS_STS_ALREADYEXISTS; } break; }
    }
    {
        tokens_t dirs_v;
        int dirs_c = split(&dirs_v, path, '/');
        fill(fs_RAPath, 0, FS_MAX_PATHLEN + 1);
        fs_RAPath[0] = '/';
        int bufptr = 1;
        for (int i = 0; i < dirs_c - 1; ++i) {
            for (int j = 0; j <= i; ++j) {
                copy(&fs_RAPath[bufptr], dirs_v[j]);
                bufptr += length(dirs_v[j]);
                fs_RAPath[bufptr] = '/';
                bufptr++;
            }
            bool parent_found = false;
            for (int j = 0; j < FS_MAX_ENTCOUNT; ++j) {
                if (fs_EntryV[j] == NULL) { continue; }
                fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[j];
                if (ent->name[0] != '\0' && compare(ent->name, fs_RAPath) == 0 && ent->type == FS_TYPE_DIR) {
                    parent_found = true;
                    break;
                }
            } if (!parent_found) { return FS_STS_PATHNOTFOUND; }
            bufptr = 1;
            fill(fs_RAPath, 0, FS_MAX_PATHLEN + 1);
            fs_RAPath[0] = '/';
        }
    }
    int index = -1;
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { index = i; break; }
    } if (index == -1) { return FS_STS_OUTOFMEMORY; }
    void* newptr = malloc(MEMORY_BLKSIZE);
    if (newptr == NULL) { return FS_STS_OUTOFMEMORY; }
    fs_EntryV[index] = newptr;
    fs_Entry_t* dir = (fs_Entry_t*)fs_EntryV[index];
    copy(dir->name, path);
    int user = 0;
    if (user == -1) { dir->user = 0; }
    else { dir->user = user; }
    int group = 0;
    if (group == -1) { dir->group = 0; }
    else { dir->group = group; }
    date(&dir->ctime);
    date(&dir->mtime);
    date(&dir->atime);
    dir->perm = 0777;
    dir->type = FS_TYPE_DIR;
    return FS_STS_SUCCESS;
}

/**
 * @brief Function for read a specific file content
 * 
 * @param path Path of specific file
 * 
 * @return Buffer of file content
 */
char* fs_readFile(const char* path) {
    if (!fs_InitLock) { return NULL; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0) {
            if (ent->type == FS_TYPE_FILE) {
                date(&fs_stat(path)->atime);
                if (fs_EntryV[i]->ftype == FS_TYPE_MOUNTED) {
                    extern void** mountmgr_MountV; if (!mountmgr_MountV) { return NULL; }
                    return (char*)mountmgr_MountV[fs_EntryV[i]->mountslot];
                } return (char*)((char*)fs_EntryV[i] + (MEMORY_BLKSIZE));
            } break;
        }
    } return NULL;
}

/**
 * @brief Function for write a file
 * 
 * @param path Path of file
 * @param size New size of file
 * @param buf Buffer for write to file
 * 
 * @return Status code
 */
int fs_writeFile(const char* path, size_t size, char* buf) {
    if (!fs_InitLock) { return FS_STS_NOTINIT; }
    if (length(path) == 0 || path[length(path) - 1] == '/') { return FS_STS_FAILURE; }
    if (buf == NULL && size != 0) { return FS_STS_FAILURE; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->type == FS_TYPE_DIR) {
            size_t ent_name_len = length(ent->name);
            if (ent_name_len > 0 && ent->name[ent_name_len - 1] == '/') {
                if ((size_t)length(path) + 1 == ent_name_len)
                    { if (ncompare(ent->name, path, length(path)) == 0) { return FS_STS_DIREXISTS; } }
            }
        }
    }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0) {
            if (ent->type == FS_TYPE_DIR) { return FS_STS_NOTFILE; }
            void* newptr = realloc(fs_EntryV[i], size + MEMORY_BLKSIZE);
            if (newptr == NULL) { return FS_STS_OUTOFMEMORY; } fs_EntryV[i] = newptr;
            ncopy((char*)fs_EntryV[i] + MEMORY_BLKSIZE, buf, size);
            ent = (fs_Entry_t*)fs_EntryV[i];
            ent->size = size; date(&ent->mtime); date(&ent->atime);
            return FS_STS_SUCCESS;
        }
    }
    {
        tokens_t dirs_v;
        int dirs_c = split(&dirs_v, path, '/');
        fill(fs_RAPath, 0, FS_MAX_PATHLEN + 1);
        fs_RAPath[0] = '/';
        int bufptr = 1;
        for (int i = 0; i < dirs_c - 1; ++i) {
            for (int j = 0; j <= i; ++j) {
                copy(&fs_RAPath[bufptr], dirs_v[j]);
                bufptr += length(dirs_v[j]);
                fs_RAPath[bufptr] = '/';
                bufptr++;
            }
            bool parent_found = false;
            for (int j = 0; j < FS_MAX_ENTCOUNT; ++j) {
                if (fs_EntryV[j] == NULL) { continue; }
                fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[j];
                if (ent->name[0] != '\0' && compare(ent->name, fs_RAPath) == 0 && ent->type == FS_TYPE_DIR) {
                    parent_found = true;
                    break;
                }
            } if (!parent_found) { return FS_STS_PATHNOTFOUND; }
            bufptr = 1;
            fill(fs_RAPath, 0, FS_MAX_PATHLEN + 1);
            fs_RAPath[0] = '/';
        }
    }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) {
            void* newptr = malloc(MEMORY_BLKSIZE + size);
            if (newptr == NULL) { return FS_STS_OUTOFMEMORY; }
            fs_EntryV[i] = newptr;
            fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
            copy(ent->name, path);
            int user = 0;
            if (user == -1) { ent->user = 0; }
            else { ent->user = user; }
            int group = 0;
            if (group == -1) { ent->group = 0; }
            else { ent->group = group; }
            ent->size = size;
            date(&ent->ctime);
            date(&ent->mtime);
            date(&ent->atime);
            ent->perm = 0777;
            ent->type = FS_TYPE_FILE;
            ent->ftype = FS_TYPE_FILE;
            ncopy((char*)fs_EntryV[i] + MEMORY_BLKSIZE, buf, size);
            return FS_STS_SUCCESS;
        }
    } return FS_STS_OUTOFMEMORY;
}

/**
 * @brief Function for remove a entry
 * 
 * @param path Path of specific entry
 * 
 * @return Status code
 */
int fs_remove(const char* path) {
    if (!fs_InitLock) { return FS_STS_NOTINIT; }
    if (compare(path, "/") == 0) { return FS_STS_FAILURE; }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && compare(ent->name, path) == 0) {
            if (ent->type == FS_TYPE_DIR) {
                int pathlen = length(path);
                fill(fs_RAPath, 0, FS_MAX_PATHLEN + 1);
                copy(fs_RAPath, path);
                if (path[pathlen - 1] != '/') {
                    fs_RAPath[pathlen] = '/';
                    fs_RAPath[pathlen + 1] = '\0';
                    pathlen++;
                }
                for (int j = 0; j < FS_MAX_ENTCOUNT; ++j) {
                    if (fs_EntryV[j] == NULL) { continue; }
                    fs_Entry_t* ent2 = (fs_Entry_t*)fs_EntryV[j];
                    if (
                        ent2->name[0] != '\0' &&
                        ncompare(ent2->name, fs_RAPath, pathlen) == 0 &&
                        length(ent2->name) > pathlen
                    ) { return FS_STS_DIRNOTEMPTY; }
                }
            } free(fs_EntryV[i]); fs_EntryV[i] = NULL; return FS_STS_SUCCESS;
        }
    } return FS_STS_ENTRYNOTFOUND;
}

/**
 * @brief Function for remove bulk files and directories
 * 
 * @param path Path of specific entry
 * 
 * @return Status code
 */
int fs_bulkRemove(const char* path) {
    if (!fs_InitLock) { return FS_STS_NOTINIT; }
    if (fs_stat(path) == NULL) { return FS_STS_PATHNOTFOUND; }
    if (fs_stat(path)->type != FS_TYPE_DIR) { return fs_remove(path); }
    for (int i = 0; i < FS_MAX_ENTCOUNT; ++i) {
        if (fs_EntryV[i] == NULL) { continue; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[i];
        if (ent->name[0] != '\0' && ent->name[0] != '\0' && ncompare(ent->name, path, length(path)) == 0) {
            free(fs_EntryV[i]); fs_EntryV[i] = NULL;
        }
    } return FS_STS_SUCCESS;
}

/**
 * @brief Function for initialize core file system
 * 
 * @return Status code
 */
int corefs_init() {
    if (fs_InitLock) { return FS_STS_FAILURE; }
    fs_EntryV = (fs_Entry_t**)calloc(FS_MAX_ENTCOUNT, sizeof(fs_Entry_t*));
    if (fs_EntryV == NULL) { PANIC("Out of memory"); }
    fs_EntryV[FS_ROOTDIR] = (fs_Entry_t*)malloc(sizeof(fs_Entry_t));
    if (fs_EntryV[FS_ROOTDIR] == NULL) { PANIC("Out of memory"); }
    fs_RADirent = (int*)malloc(FS_MAX_ENTCOUNT * sizeof(int));
    if (fs_RADirent == NULL) { PANIC("Out of memory"); }
    fs_RAPath = (char*)malloc(FS_MAX_PATHLEN);
    if (fs_RAPath == NULL) { PANIC("Out of memory"); }
    fs_Entry_t* rootdir = (fs_Entry_t*)fs_EntryV[FS_ROOTDIR];
    copy(rootdir->name, "/");
    rootdir->user = 0;
    rootdir->group = 0;
    date(&rootdir->ctime);
    date(&rootdir->mtime);
    date(&rootdir->atime);
    rootdir->perm = 0777;
    rootdir->type = FS_TYPE_DIR;
    fs_InitLock = true; return FS_STS_SUCCESS;
}