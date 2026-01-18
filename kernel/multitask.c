#include "kernel.h"

// * Imports

// Imported context switch function from multitask_switch.s
extern void multitask_switch(multitask_Context_t* old, multitask_Context_t* new);

// * Variables

// Multitasking initialize state (false at startup)
bool multitask_Initialized = false;

// Process table pointer
multitask_Process_t* multitask_ProcessTable;

// Process table size
size_t multitask_ProcessTableSize = MULTITASK_ENTCOUNT * sizeof(multitask_Process_t);

// Currently active process ID (0 at startup)
pid_t multitask_Focus = 0;

// Default register values for new processes (Filled after initialization)
multitask_Context_t multitask_DefaultRegs;

// List of users
multitask_User_t multitask_UserList[MULTITASK_USER_COUNTMAX];

// List of groups
char multitask_GroupList[MULTITASK_USER_COUNTMAX][MULTITASK_USER_NAMELEN + 1];

// Buffer of active processes list
char* multitask_ReportBuffer = NULL;

// * Functions

/**
 * @brief Function for initialize multitasking system
 */
void multitask_init() {
    if (multitask_Initialized) { return; }
    multitask_ProcessTable = (multitask_Process_t*)malloc(multitask_ProcessTableSize);
    if (multitask_ProcessTable == NULL) { PANIC("Multitasking initialization failed, unable to allocate memory"); }
    fill(multitask_ProcessTable, 0, multitask_ProcessTableSize);
    for (int i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
        fill(multitask_UserList[i].name, 0, MULTITASK_USER_NAMELEN + 1);
        for (int j = 0; j < MULTITASK_USER_GROUPMAX; ++j) { multitask_UserList[i].group[j] = MULTITASK_USER_NULL; }
        fill(multitask_UserList[i].home, 0, MULTITASK_USER_NAMELEN + 1);
        multitask_UserList[i].pass = 0; multitask_UserList[i].active = false;
        fill(multitask_GroupList[i], 0, MULTITASK_NAMELEN + 1);
    }
    // root
    copy(multitask_UserList[MULTITASK_USER_ROOT].name, "root");
    copy(multitask_GroupList[MULTITASK_USER_ROOT], "root");
    multitask_UserList[MULTITASK_USER_ROOT].group[MULTITASK_USER_ROOT] = MULTITASK_USER_ROOT;
    copy(multitask_UserList[MULTITASK_USER_ROOT].home, "/root/");
    multitask_UserList[MULTITASK_USER_ROOT].pass = fnv1ahash("", 0);
    multitask_UserList[MULTITASK_USER_ROOT].active = true;
    // daemon
    copy(multitask_UserList[MULTITASK_USER_DAEMON].name, "daemon");
    copy(multitask_GroupList[MULTITASK_USER_DAEMON], "daemon");
    multitask_UserList[MULTITASK_USER_DAEMON].group[0] = MULTITASK_USER_DAEMON;
    copy(multitask_UserList[MULTITASK_USER_DAEMON].home, "");
    multitask_UserList[MULTITASK_USER_DAEMON].pass = fnv1ahash("", 0);
    multitask_UserList[MULTITASK_USER_DAEMON].active = true;
    // users
    copy(multitask_GroupList[MULTITASK_USER_USERS], "users");
    // sudo
    copy(multitask_GroupList[MULTITASK_USER_SUDO], "sudo");
    // nobody & nogroup
    copy(multitask_UserList[MULTITASK_USER_NOBODY].name, "nobody");
    multitask_UserList[MULTITASK_USER_NOBODY].group[0] = MULTITASK_USER_NOBODY;
    copy(multitask_GroupList[MULTITASK_USER_NOBODY], "nogroup");
    copy(multitask_UserList[MULTITASK_USER_NOBODY].home, "");
    multitask_UserList[MULTITASK_USER_NOBODY].pass = fnv1ahash("", 0);
    multitask_UserList[MULTITASK_USER_NOBODY].active = true;
    asm volatile("movl %%cr3, %%eax\t\n movl %%eax, %0":"=m"(multitask_DefaultRegs.CR3)::"%eax");
    asm volatile("pushfl\t\n movl (%%esp), %%eax\t\n movl %%eax, %0\t\n popfl":"=m"(multitask_DefaultRegs.EFLAGS)::"%eax");
    multitask_Initialized = true;
}

/**
 * @brief Function for get list of active processes
 * 
 * @return Buffer of active processes list
 */
char* multitask_report() {
    if (!multitask_Initialized) { return NULL; }
    if (multitask_ReportBuffer == NULL) {
        multitask_ReportBuffer = (char*)malloc(MEMORY_BLOCKSIZE);
        if (multitask_ReportBuffer == NULL) { return NULL; }
    } fill (multitask_ReportBuffer, 0, MEMORY_BLOCKSIZE); int ptr = 0;
    for (int i = 1; i < MULTITASK_ENTCOUNT; ++i) {
        if (!multitask_ProcessTable[i].active) { continue; }
        int n = snprintf(
            &multitask_ReportBuffer[ptr], MULTITASK_USER_NAMELEN + MULTITASK_NAMELEN + 8, "%s\t%d\t%d\t%s\n",
            multitask_UserList[multitask_ProcessTable[i].user].name,
            i, multitask_ProcessTable[i].parent, multitask_ProcessTable[i].name
        ); ptr += n;
    } return multitask_ReportBuffer;
}

/**
 * @brief Function for spawn a new process
 * 
 * @param name Name for new process
 * @param program Program pointer for new process
 * 
 * @return Process ID of new process
 */
pid_t spawn(char* name, void* program) {
    if (!multitask_Initialized || program == NULL) { return -1; }
    int pid = 0; for (int i = 1; i < MULTITASK_ENTCOUNT; ++i) {
        if (!multitask_ProcessTable[i].active) { pid = i; break; }
    } if (pid == 0) { return -1; }
    fill(multitask_ProcessTable[pid].name, 0, MULTITASK_NAMELEN);
    if (name != NULL) {
        if (length(name) < MULTITASK_NAMELEN) {
            copy(multitask_ProcessTable[pid].name, name);
        } else { ncopy(multitask_ProcessTable[pid].name, name, MULTITASK_NAMELEN - 1); }
    } else { copy(multitask_ProcessTable[pid].name, "[Unknown]"); }
    if (multitask_Focus == 0) { multitask_ProcessTable[pid].user = 0; }
    else { multitask_ProcessTable[pid].user = multitask_ProcessTable[multitask_Focus].user; }
    multitask_ProcessTable[pid].parent = multitask_Focus;
    multitask_ProcessTable[pid].context.EAX = 0;
    multitask_ProcessTable[pid].context.EBX = 0;
    multitask_ProcessTable[pid].context.ECX = 0;
    multitask_ProcessTable[pid].context.EDX = 0;
    multitask_ProcessTable[pid].context.ESI = 0;
    multitask_ProcessTable[pid].context.EDI = 0;
    multitask_ProcessTable[pid].context.EFLAGS = multitask_DefaultRegs.EFLAGS;
    multitask_ProcessTable[pid].context.EIP = (size_t)program;
    multitask_ProcessTable[pid].context.CR3 = multitask_DefaultRegs.CR3;
    multitask_ProcessTable[pid].context.ESP = (size_t)malloc(MEMORY_BLOCKSIZE) + MEMORY_BLOCKSIZE;
    multitask_ProcessTable[pid].active = true;
    return pid;
}

/**
 * @brief Function for kill a process
 * 
 * @param pid Target process ID to kill
 * 
 * @return Operation status (-1 means failure)
 */
int kill(pid_t pid) {
    if (!multitask_Initialized || pid <= 1 || pid >= MULTITASK_ENTCOUNT ||
        (multitask_Focus != 0 && multitask_ProcessTable[multitask_Focus].user != MULTITASK_USER_ROOT)) { return -1; }
    free((void*)(multitask_ProcessTable[pid].context.ESP - MEMORY_BLOCKSIZE));
    fill(multitask_ProcessTable[pid].name, 0, MULTITASK_NAMELEN);
    fill(&multitask_ProcessTable[pid].context, 0, sizeof(multitask_Context_t));
    multitask_ProcessTable[pid].active = false; return 0;
}

/**
 * @brief Function for switch to next process
 */
void yield() {
    if (!multitask_Initialized) { return; }
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    int next = 0;
    if (multitask_Focus == 0) {
        for (int i = 1; i < MULTITASK_ENTCOUNT; ++i) {
            if (multitask_ProcessTable[i].active) { next = i; break; }
        } if (next == 0) { PANIC("No processes to execute"); }
    } else {
        for (int i = multitask_Focus + 1; i < MULTITASK_ENTCOUNT; ++i) {
            if (multitask_ProcessTable[i].active) { next = i; break; }
        } if (next == 0) {
            for (int i = 1; i < MULTITASK_ENTCOUNT; ++i) {
                if (multitask_ProcessTable[i].active) { next = i; break; }
            } if (next == 0) { PANIC("No processes to execute"); }
        }
    } int old = multitask_Focus; multitask_Focus = next; if (old == next) { return; }
    multitask_switch(&multitask_ProcessTable[old].context, &multitask_ProcessTable[next].context);
}

/**
 * @brief Function for end current process
 */
void exit() {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0) { return; }
    kill(multitask_Focus); yield(); return;
}

/**
 * @brief Function for add new user
 * 
 * @param name Username
 * 
 * @return Operation status
 */
int uadd(char* name) {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (multitask_Focus != 0) { if (getuid() != MULTITASK_USER_ROOT) { return -1; } }
    if (!multitask_Initialized || name == NULL ||
        length(name) < 1 || length(name) > MULTITASK_USER_NAMELEN) { return -1; }
    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
        if (!multitask_UserList[i].active) { continue; }
        if (compare(name, multitask_UserList[i].name) == 0) { return -1; }
    }
    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
        if (!multitask_UserList[i].active) {
            copy(multitask_UserList[i].name, name);
            multitask_UserList[i].group[0] = MULTITASK_USER_USERS;
            multitask_UserList[i].pass = fnv1ahash("", 0);
            multitask_UserList[i].active = true; return 0;
        }
    } return -1;
}

/**
 * @brief Function for delete a user
 * 
 * @param name Username
 * 
 * @return Operation status
 */
int udel(char* name) {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (multitask_Focus != 0) { if (getuid() != MULTITASK_USER_ROOT) { return -1; } }
    if (!multitask_Initialized || name == NULL ||
        length(name) < 1 || length(name) > MULTITASK_USER_NAMELEN) { return -1; }
    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
        if (!multitask_UserList[i].active) { continue; }
        if (compare(name, multitask_UserList[i].name) != 0) { continue; }
        if (i == MULTITASK_USER_ROOT ||
            i == MULTITASK_USER_DAEMON ||
            i == MULTITASK_USER_NOBODY) { return -1; }
        fill(&multitask_UserList[i], 0, sizeof(multitask_User_t));
        for (gid_t j = 0; j < MULTITASK_USER_GROUPMAX; ++j) {
            multitask_UserList[i].group[j] = MULTITASK_USER_NULL;
        } return 0;
    } return -1;
}

/**
 * @brief Function for change user password
 * 
 * @param name Username
 * @param oldp New password
 * @param newp Old password
 * 
 * @return Operation status
 */
int upwd(char* name, char* oldp, char* newp) {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0 || name == NULL ||
        length(name) < 1 || length(name) > MULTITASK_USER_NAMELEN) { return -1; }
    for (uid_t i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
        if (!multitask_UserList[i].active) { continue; }
        if (compare(name, multitask_UserList[i].name) != 0) { continue; }
        if (multitask_Focus != 0) { if (getuid() != MULTITASK_USER_ROOT && getuid() != i) { return -1; } }
        if (multitask_Focus == 0 || getuid() == MULTITASK_USER_ROOT) {
            multitask_UserList[i].pass = fnv1ahash(newp, newp ? length(newp) : 0); return 0;
        }
        if (multitask_UserList[i].pass == fnv1ahash(oldp, oldp ? length(oldp) : 0)) {
            multitask_UserList[i].pass = fnv1ahash(newp, newp ? length(newp) : 0); return 0;
        } return -1;
    } return -1;
}

/**
 * @brief Function for switch another user
 * 
 * @param name Username
 * @param pass Password
 * 
 * @return Operation status
 */
int uswi(char* name, char* pass) {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0 || name == NULL ||
        length(name) < 1 || length(name) > MULTITASK_USER_NAMELEN) { return -1; }
    for (int i = 0; i < MULTITASK_USER_COUNTMAX; ++i) {
        if (!multitask_UserList[i].active) { continue; }
        if (compare(name, multitask_UserList[i].name) == 0) {
            if (multitask_ProcessTable[multitask_Focus].user != 0) {
                uint32_t entry = fnv1ahash(pass, pass ? length(pass) : 0);
                if (entry == multitask_UserList[i].pass) {
                    multitask_ProcessTable[multitask_Focus].user = i; return 0;
                } else { return -1; }
            } else { multitask_ProcessTable[multitask_Focus].user = i; return 0; }
        }
    } return -1;
}

/**
 * @brief Function for get current process ID
 * 
 * @return Process ID
 */
pid_t getpid() {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0) { return -1; } return multitask_Focus;
}

/**
 * @brief Function for get current parent process ID
 * 
 * @return Parent process ID
 */
pid_t getppid() {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0) { return -1; }
    return multitask_ProcessTable[multitask_Focus].parent;
}

/**
 * @brief Function for get current user ID
 * 
 * @return User ID
 */
uid_t getuid() {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0) { return -1; }
    return multitask_ProcessTable[multitask_Focus].user;
}

/**
 * @brief Function for get current group ID
 * 
 * @return Group ID
 */
gid_t getgid() {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_ENTCOUNT) { multitask_Focus = 0; }
    if (!multitask_Initialized || multitask_Focus == 0) { return -1; }
    return multitask_UserList[multitask_ProcessTable[multitask_Focus].user].group[0];
}