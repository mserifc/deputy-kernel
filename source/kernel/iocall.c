#include "kernel.h"

// * Constants

// Limit of active file descriptors
#define IOCALL_MAXFD 64

// * Imports

// Imported file system entry table from ramfs
extern fs_Entry_t** fs_EntryV;

// * Structures

// Structure of file descriptor
typedef struct {
    int entry;      // Entry index in file system
    int flags;      // Access and control flags
    size_t ptr;     // Pointer for read/write syscalls
    bool op;        // Operation status
} iocall_FileDesc_t;

// * Variables

// File descriptor table
iocall_FileDesc_t iocall_FileDesc[IOCALL_MAXFD];

// I/O system calls initialize state
bool iocall_Initialized = false;

// Access file forcefully if this flag set (only for kernel components)
bool iocall_ForceAccess = false;

// * Functions

/**
 * @brief System call for read data from a specific file descriptor
 * 
 * @param fd File descriptor
 * @param buf Buffer to store the read data
 * @param count Number of bytes to read
 * 
 * @return Number of bytes read (-1 means failure)
 */
size_t read(int fd, void* buf, size_t count) {
    // if (fd == STDIN) {
    //     char* str = (char*)buf; uint32_t ptr = 0;
    //     char* chardev = fs_readFile("/dev/keyboard");
    //     if (chardev == NULL) { return -1; }
    //     uint32_t* bufsize = (uint32_t*)chardev;
    //     uint32_t limit = 0;
    //     if (bufsize[0] >= count) { limit = count; } else if (bufsize[0] < count) { limit = bufsize[0]; }
    //     for (int i = 0; i < limit; ++i) {
    //         uint8_t key = chardev[sizeof(uint32_t) + i];
    //         switch (key) {
    //             case KEYBOARD_KEY_SPECIAL: { continue; }
    //             case KEYBOARD_KEY_CAPSLOCK: {
    //                 if (keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK]) {
    //                     keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] = false;
    //                 } else {
    //                     keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] = true;
    //                 } continue;
    //             }
    //             case KEYBOARD_KEY_LSHIFT: { keyboard_KeyState[KEYBOARD_KEY_LSHIFT] = true; continue; }
    //             case (KEYBOARD_KEY_LSHIFT + KEYBOARD_KEY_RELEASE):
    //                 { keyboard_KeyState[KEYBOARD_KEY_LSHIFT] = false; continue; }
    //             case KEYBOARD_KEY_RSHIFT: { keyboard_KeyState[KEYBOARD_KEY_RSHIFT] = true; continue; }
    //             case (KEYBOARD_KEY_RSHIFT + KEYBOARD_KEY_RELEASE):
    //                 { keyboard_KeyState[KEYBOARD_KEY_RSHIFT] = false; continue; }
    //             default: {
    //                 if (key >= KEYBOARD_KEY_ESCAPE + KEYBOARD_KEY_RELEASE &&
    //                     key <= KEYBOARD_KEY_F12 + KEYBOARD_KEY_RELEASE) { continue; }
    //                 bool shift = false; if (
    //                     keyboard_KeyState[KEYBOARD_KEY_CAPSLOCK] ||
    //                     keyboard_KeyState[KEYBOARD_KEY_LSHIFT] ||
    //                     keyboard_KeyState[KEYBOARD_KEY_RSHIFT]) { shift = true; }
    //                 str[ptr] = (char)keyboard_Layout_US[key][shift]; ++ptr;
    //             }
    //         }
    //     } if (bufsize[0] > limit) {
    //         uint32_t remain = bufsize[0] - limit;
    //         ncopy(&chardev[sizeof(uint32_t)], &chardev[sizeof(uint32_t) + limit], remain);
    //         fill(&chardev[sizeof(uint32_t) + remain], 0, MEMORY_BLKSIZE - sizeof(uint32_t) - remain);
    //         bufsize[0] = remain;
    //     } else { bufsize[0] = 0; fill(&chardev[sizeof(uint32_t)], 0, MEMORY_BLKSIZE - sizeof(uint32_t)); }
    //     return ptr;
    // } else
    if (fd >= TYPEFD && fd < TYPEFD + IOCALL_MAXFD) {
        int fdesc = TYPEFD - fd; if (iocall_FileDesc[fdesc].entry == 0) { return -1; }
        if (!iocall_ForceAccess) {
            if (!(iocall_FileDesc[fdesc].flags & O_RDONLY) && !(iocall_FileDesc[fdesc].flags & O_RDWR)) { return -1; }
        }
        if (fs_EntryV[iocall_FileDesc[fdesc].entry] == NULL) { return -1; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[iocall_FileDesc[fdesc].entry];
        if (ent->type != FS_TYPE_FILE) { return -1; } char* str = (char*)buf;
        if (ent->ftype == FS_TYPE_FILE) {
            if (iocall_FileDesc[fdesc].ptr >= ent->size) { return 0; }
            char* data = fs_readFile(ent->name); if (data == NULL) { return -1; }
            size_t remain = ent->size - iocall_FileDesc[fdesc].ptr;
            size_t limit = 0; if (count > remain) { limit = remain; } else { limit = count; }
            for (size_t i = 0; i < limit; ++i) {
                str[i] = data[iocall_FileDesc[fdesc].ptr + i];
            } iocall_FileDesc[fdesc].ptr += limit; return limit;
        } else if (ent->ftype == FS_TYPE_CHARDEV) {
            if (!iocall_ForceAccess) {
                if (!(fs_stat(ent->name)->devperm & O_RDONLY) &&
                    !(fs_stat(ent->name)->devperm & O_RDWR)) { return -1; }
            }
            char* chardev = fs_readFile(ent->name); if (chardev == NULL) { return -1; }
            uint32_t* bufsize = (uint32_t*)chardev; uint32_t limit = 0;
            if (bufsize[0] >= count) { limit = count; } else if (bufsize[0] < count) { limit = bufsize[0]; }
            for (size_t i = 0; i < limit; ++i) { str[i] = chardev[sizeof(uint32_t) + i]; }
            if (bufsize[0] > limit) {
                uint32_t remain = bufsize[0] - limit;
                ncopy(&chardev[sizeof(uint32_t)], &chardev[sizeof(uint32_t) + limit], remain);
                fill(&chardev[sizeof(uint32_t) + remain], 0, ent->size - sizeof(uint32_t) - remain);
                bufsize[0] = remain;
            } else { bufsize[0] = 0; fill(&chardev[sizeof(uint32_t)], 0, ent->size - sizeof(uint32_t)); }
            return limit;
        }
    } return -1;
}

/**
 * @brief System call for write data to specific file descriptor
 * 
 * @param fd File descriptor
 * @param buf Data buffer to be written
 * @param count Number of bytes to write
 * 
 * @return Number of bytes written (-1 means failure)
 */
size_t write(int fd, void* buf, size_t count) {
    // if (fd == STDOUT) {
    //     char* str = (char*)buf; int c = 0;
    //     for (int i = 0; i < count; ++i) { putchar(str[i]); ++c; }
    //     return c;
    // } else
    if (fd >= TYPEFD && fd < TYPEFD + IOCALL_MAXFD) {
        int fdesc = fd - TYPEFD; if (iocall_FileDesc[fdesc].entry == 0) { return -1; }
        if (!iocall_ForceAccess) {
            if (!(iocall_FileDesc[fdesc].flags & O_WRONLY) && !(iocall_FileDesc[fdesc].flags & O_RDWR)) { return -1; }
        }
        if (fs_EntryV[iocall_FileDesc[fdesc].entry] == NULL) { return -1; }
        fs_Entry_t* ent = (fs_Entry_t*)fs_EntryV[iocall_FileDesc[fdesc].entry];
        if (ent->type != FS_TYPE_FILE) { return -1; } char* str = (char*)buf;
        if (ent->ftype == FS_TYPE_FILE) { size_t limit = 0;
            if (ent->size >= iocall_FileDesc[fdesc].ptr + count) { limit = ent->size; }
            else { limit = iocall_FileDesc[fdesc].ptr + count; }
            char* file = fs_readFile(ent->name); if (file == NULL) { return -1; }
            char* data = (char*)malloc(limit); if (data == NULL) { return -1; } fill(data, 0, limit);
            ncopy(data, file, ent->size); ncopy(&data[iocall_FileDesc[fdesc].ptr], buf, count);
            if (fs_writeFile(ent->name, limit, data) != FS_STS_SUCCESS) { free(data); return -1; }
            iocall_FileDesc[fdesc].ptr += count; free(data); return count;
        } else if (ent->ftype == FS_TYPE_CHARDEV) {
            if (!iocall_ForceAccess) {
                if (!(fs_stat(ent->name)->devperm & O_WRONLY) &&
                    !(fs_stat(ent->name)->devperm & O_RDWR)) { return -1; }
            }
            char* chardev = fs_readFile(ent->name); if (chardev == NULL) { return -1; }
            uint32_t* bufsize = (uint32_t*)chardev; char* str = (char*)buf;
            size_t c = 0; for (size_t i = 0; i < count; ++i) {
                if (bufsize[0] < ent->size - sizeof(uint32_t)) {
                    chardev[sizeof(uint32_t) + bufsize[0]] = str[i]; ++bufsize[0]; ++c;
                }
            } return c;
        }
    } return -1;
}

/**
 * @brief System call for open a file descriptor
 * 
 * @param path Path of file to open
 * @param flags Access and control flags
 * 
 * @return File descriptor (-1 means failure)
 */
int open(char* path, int flags) {
    if (!iocall_Initialized) { for (int i = 0; i < IOCALL_MAXFD; ++i) {
        iocall_FileDesc[i].entry = 0; iocall_FileDesc[i].flags = 0;
        iocall_FileDesc[i].ptr = 0; iocall_FileDesc[i].op = false; } }
    bool created = false; int index = fs_index(path); if (index == FS_STS_ENTRYNOTFOUND) {
        if (flags & O_CREAT) {
            if (!iocall_ForceAccess) {
                if (0/*!fs_checkPerm(fs_parent(path), FS_PERM_WRITE)*/) { return -1; }
            }
            if (fs_writeFile(path, 0, "") != FS_STS_SUCCESS) { return -1; }
            created = true; index = fs_index(path); if (index == FS_STS_ENTRYNOTFOUND) { return -1; }
        } else { return -1; }
    } else if (index < 1 || index >= FS_MAX_ENTCOUNT) { return -1; }
    if (fs_stat(path)->type != FS_TYPE_FILE) { return -1; }
    if (!iocall_ForceAccess) {
        if (fs_stat(path)->ftype == FS_TYPE_CHARDEV && !(flags & O_RDWR)) {
            if (flags & O_RDONLY) {
                if (fs_stat(path)->devperm & O_WRONLY) { return -1; }
            } else if (flags & O_WRONLY)
                { if (fs_stat(path)->devperm & O_RDONLY) { return -1; } }
        }
    }
    if (flags & O_EXCL && !created) { return -1; }
    if (flags & O_TRUNC && !created) {
        if (!iocall_ForceAccess) {
            if (0/*!fs_checkPerm(fs_stat(path), FS_PERM_WRITE)*/) { return -1; }
        }
        if (fs_writeFile(path, 0, "") != FS_STS_SUCCESS) { return -1; }
    }
    if (!iocall_ForceAccess) {
        if (flags & O_APPEND && flags & O_RDONLY && !(flags & O_RDWR)) { return -1; }
        if (((flags & O_RDWR) || (flags & O_RDONLY)) && 0/*!fs_checkPerm(fs_stat(path), FS_PERM_READ)*/) { return -1; }
        if (((flags & O_RDWR) || (flags & O_WRONLY)) && 0/*!fs_checkPerm(fs_stat(path), FS_PERM_WRITE)*/) { return -1; }
    }
    int found = -1; for (int i = 0; i < IOCALL_MAXFD; ++i) {
        if (iocall_FileDesc[i].entry == 0) { found = i; break; }
    } if (found == -1) { return -1; }
    iocall_FileDesc[found].entry = index; iocall_FileDesc[found].flags = flags; return TYPEFD + found;
}

/**
 * @brief System call for close a file descriptor
 * 
 * @param fd Specific file descriptor
 * 
 * @return Operation status
 */
int close(int fd) {
    if (fd < TYPEFD || fd >= TYPEFD + IOCALL_MAXFD ||
        iocall_FileDesc[fd].entry == 0 ||
        iocall_FileDesc[fd].op == true) { return -1; }
    iocall_FileDesc[fd].entry = 0;
    iocall_FileDesc[fd].flags = 0;
    iocall_FileDesc[fd].ptr = 0;
    return 0;
}