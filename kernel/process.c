/**
 * ! The process manager failed. A new method will be tried after new build
 */

#include "kernel.h"

#define PROCESS_MAXLIMIT    32      // Maximum limit of processes
#define PROCESS_NAMELEN     16      // Maximum length of process name

size_t process_LastInst = 0x0000;

// Structure of process context
typedef struct {
    uint32_t EAX, EBX, ECX, EDX, ESI, EDI, ESP, EFLAGS;
} process_Context_t;

// Structure of process
typedef struct {
    char name[PROCESS_NAMELEN];
    bool active;
    process_Context_t context;
    void* base;
    void* inst;
} process_t;

// Process manager initialize state
bool process_Initialized = false;

// Process table pointer
process_t* process_Table;

// Process table size
size_t process_TableSize = PROCESS_MAXLIMIT * sizeof(process_t);

// Currently focused process ID
int process_Focus = 0;

/**
 * @brief Function for initialize process manager
 */
void process_init() {
    if (process_Initialized) { return; }
    process_Table = malloc(process_TableSize);
    if (process_Table == NULL) { kernel_panic("Process manager initialization failed, unable to allocate memory"); }
    fill(process_Table, 0, process_TableSize);
    process_Initialized = true; return;
}

/**
 * @brief Function for spawn a process
 * 
 * @param name Name of process
 * @param program Pointer of program
 * 
 * @return Process ID
 */
int spawn(char* name, void* program) {
    if (!process_Initialized || program == NULL) { return -1; }
    int pid = 0; for (int i = 1; i < PROCESS_MAXLIMIT; ++i) {
        if (!process_Table[i].active) { pid = i; break; }
    } if (pid == 0) { return -1; }
    fill(process_Table[pid].name, 0, PROCESS_NAMELEN);
    if (name != NULL) {
        if (length(name) < PROCESS_NAMELEN) {
            copy(process_Table[pid].name, name);
        } else { ncopy(process_Table[pid].name, name, PROCESS_NAMELEN); }
    } else { copy(process_Table[pid].name, "[Unknown]"); }
    process_Table[pid].active = true;
    process_Table[pid].context.EAX = 0;
    process_Table[pid].context.EBX = 0;
    process_Table[pid].context.ECX = 0;
    process_Table[pid].context.EDX = 0;
    process_Table[pid].context.ESI = 0;
    process_Table[pid].context.EDI = 0;
    asm volatile ("mov %%esp, %0" : "=r"(process_Table[pid].context.ESP));
    process_Table[pid].context.EFLAGS = 0x00000002;
    process_Table[pid].base = program;
    process_Table[pid].inst = program;
    return pid;
}

/**
 * @brief Function for kill a process
 * 
 * @param pid Target process ID
 * 
 * @return Operation status
 */
int kill(int pid) {
    if (!process_Initialized || pid < 1 || pid >= PROCESS_MAXLIMIT) { return -1; }
    fill(process_Table[pid].name, 0, PROCESS_NAMELEN);
    fill(&process_Table[pid].context, 0, sizeof(process_Context_t));
    process_Table[pid].active = false;
    process_Table[pid].base = NULL;
    process_Table[pid].inst = NULL;
    return 0;
}

/**
 * @brief Function for switch next process
 */
void yield() {
    if (!process_Initialized) { return; }
    void* inst = __builtin_return_address(0);
    if (process_Focus < 0 || process_Focus >= PROCESS_MAXLIMIT) { process_Focus = 0; }
    int pid = 0;
    if (process_Focus == 0) {
        for (int i = 1; i < PROCESS_MAXLIMIT; ++i) {
            if (process_Table[i].active) {
                process_Focus = i;
                asm volatile (
                    "mov %0, %%eax\t\n"
                    "mov %1, %%ebx\t\n"
                    "mov %2, %%ecx\t\n"
                    "mov %3, %%edx\t\n"
                    "mov %4, %%esi\t\n"
                    "mov %5, %%edi\t\n"
                    : : "m"(process_Table[i].context.EAX),
                        "m"(process_Table[i].context.EBX),
                        "m"(process_Table[i].context.ECX),
                        "m"(process_Table[i].context.EDX),
                        "m"(process_Table[i].context.ESI),
                        "m"(process_Table[i].context.EDI)
                );
                asm volatile (
                    "push %0\t\n"
                    "popf\t\n"
                    : : "r"(process_Table[i].context.EFLAGS)
                );
                asm volatile ("mov %0, %%esp" : : "r"(process_Table[i].context.ESP));
                asm volatile ("jmp *%0" : : "r"(process_Table[i].inst));
            }
        } kernel_panic("No processes to execute");
    } else {
        asm volatile (
            "mov %%eax, %0\t\n"
            "mov %%ebx, %1\t\n"
            "mov %%ecx, %2\t\n"
            "mov %%edx, %3\t\n"
            "mov %%esi, %4\t\n"
            "mov %%edi, %5\t\n"
            : "=m"(process_Table[process_Focus].context.EAX),
              "=m"(process_Table[process_Focus].context.EBX),
              "=m"(process_Table[process_Focus].context.ECX),
              "=m"(process_Table[process_Focus].context.EDX),
              "=m"(process_Table[process_Focus].context.ESI),
              "=m"(process_Table[process_Focus].context.EDI)
        );
        asm volatile (
            "pushf\t\n"
            "pop %0\t\n"
            : "=r"(process_Table[process_Focus].context.EFLAGS)
        );
        asm volatile ("mov %%esp, %0" : "=m"(process_Table[process_Focus].context.ESP));
        process_Table[process_Focus].inst = inst;
        for (int i = process_Focus + 1; i < PROCESS_MAXLIMIT; ++i) {
            if (process_Table[i].active) { pid = i; break; }
        } if (pid == 0) {
            for (int i = 1; i < PROCESS_MAXLIMIT; ++i) {
                if (process_Table[i].active) { pid = i; break; }
            } if (pid == 0) { kernel_panic("No processes to execute"); }
        } process_Focus = pid;
        asm volatile (
            "mov %0, %%eax\t\n"
            "mov %1, %%ebx\t\n"
            "mov %2, %%ecx\t\n"
            "mov %3, %%edx\t\n"
            "mov %4, %%esi\t\n"
            "mov %5, %%edi\t\n"
            : : "m"(process_Table[pid].context.EAX),
                "m"(process_Table[pid].context.EBX),
                "m"(process_Table[pid].context.ECX),
                "m"(process_Table[pid].context.EDX),
                "m"(process_Table[pid].context.ESI),
                "m"(process_Table[pid].context.EDI)
        );
        asm volatile (
            "push %0\t\n"
            "popf\t\n"
            : : "r"(process_Table[pid].context.EFLAGS)
        );
        asm volatile ("mov %0, %%esp" : : "r"(process_Table[pid].context.ESP));
        asm volatile ("jmp *%0" : : "r"(process_Table[pid].inst));
    } __builtin_unreachable();
}

/**
 * @brief Function for end current process
 */
void exit() {
    if (process_Focus < 0 || process_Focus >= PROCESS_MAXLIMIT) { process_Focus = 0; }
    if (!process_Initialized || process_Focus == 0) { return; }
    kill(process_Focus); yield(); return;
}