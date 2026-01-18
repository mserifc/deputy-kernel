#include "kernel.h"

// * Imports

// Imported context switch function from multitask_swi.s
extern void multitask_swi(void* old, void* next);

// * Constants

#define MULTITASK_PROCLIMIT     32              // Process limit
#define MULTITASK_NAMELIMIT     16              // Length limit for process name
#define MULTITASK_STACKSIZE     (4 * 1024)      // Stack size for processes

#define MULTITASK_PROGMAGIC     0x464C457F      // Magic number of program files ("\x7FELF")
#define MULTITASK_PROGPTLOAD    1

// * Types and structures

// Structure of context for save/restore registers
typedef struct {
    uint32_t
        EAX, EBX, ECX, EDX,
        ESI, EDI, ESP, EBP,
        EIP, EFLAGS, CR3;
} multitask_Ctx_t;

// Structure of process
typedef struct {
    char name[MULTITASK_NAMELIMIT];     // Name of process
    multitask_Ctx_t context;            // Context structure
    void* stack;                        // Stack memory base pointer
    int parent; int user;               // Parent process and owner user
    bool file; bool freeze; bool active;    // Status
} multitask_Proc_t;

// Structure of 32-bit ELF file header
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} PACKED multitask_ProgELF32EH_t;

// Structure of 32-bit ELF program header
typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} PACKED multitask_ProgELF32PH_t;

// * Variables and tables

// Initialize lock for prevent re-initializing multitasking system
bool multitask_InitLock = false;

// Active if the process stream started
bool multitask_InStream = false;

// Kernel process structure
multitask_Proc_t multitask_KernelProc;

// Currently active process ID (0 at startup)
int multitask_Focus = 0;

// Process vector
multitask_Proc_t* multitask_ProcV;

// Default register values for new processes (Filled after initialization)
multitask_Ctx_t multitask_DefRegs;

// * Subfunctions

// A sentry for oversee target process
void sentry(int pid) {
    if (!multitask_InitLock || pid < 1 || pid >= MULTITASK_PROCLIMIT || !multitask_ProcV[pid].active) { return; }
    multitask_Proc_t proc; ncopy(&proc, &multitask_ProcV[pid], sizeof(multitask_Proc_t));
    bool execution = true; char* reason;
    if (multitask_ProcV[pid].context.ESP <= (size_t)multitask_ProcV[pid].stack) {
        reason = "stack explosion";
    } else if (multitask_ProcV[pid].context.ESP > ((size_t)multitask_ProcV[pid].stack + MULTITASK_STACKSIZE)) {
        reason = "stack implosion";
    } else { execution = false; } if (execution) {
        if (kill(pid) != -1) {
            INFO("Process %d (%s) killed - %s", pid, proc.name, reason);
        } else {
            multitask_ProcV[pid].freeze = true;
            INFO("Undying process %d (%s) freezed - %s", pid, proc.name, reason);
        }
    }
    // if (multitask_ProcV[pid].context.ESP <= (size_t)multitask_ProcV[pid].stack) {
    //     if (kill(pid) != -1) { INFO("Process %d (%s) killed - stack explosion (%d bytes damaged)",
    //         pid, proc.name, (size_t)proc.stack - proc.context.ESP); }
    //     else {
    //         if (!multitask_ProcV[pid].freeze) {
    //             multitask_ProcV[pid].freeze = true;
    //             INFO("Undying process %d (%s) freezed - stack explosion (%d bytes damaged)",
    //                 pid, proc.name, (size_t)proc.stack - proc.context.ESP);
    //         }
    //     }
    // }
}

// * Functions

/**
 * @brief Function for spawn a new process
 * 
 * @param name Name for new process
 * @param prog Program pointer for new process
 * 
 * @return Process ID of new process (-1 means failure)
 */
int spawn(const char* name, func_t prog) {
    if (!multitask_InitLock || prog == NULL) { return -1; }
    int pid = 0; for (int i = 1; i < MULTITASK_PROCLIMIT; ++i) {
        if (!multitask_ProcV[i].active) { pid = i; break; }
    } if (pid == 0) { return -1; }
    multitask_ProcV[pid].stack = malloc(MULTITASK_STACKSIZE);
    if (multitask_ProcV[pid].stack == NULL) { return -1; }
    fill(multitask_ProcV[pid].name, 0, MULTITASK_NAMELIMIT);
    if (name != NULL) {
        if (length(name) < MULTITASK_NAMELIMIT) {
            copy(multitask_ProcV[pid].name, name);
        } else { ncopy(multitask_ProcV[pid].name, name, MULTITASK_NAMELIMIT - 1); }
    } else { copy(multitask_ProcV[pid].name, "[Unknown]"); }
    multitask_ProcV[pid].parent = 0;
    multitask_ProcV[pid].context.EAX = 0;
    multitask_ProcV[pid].context.EBX = 0;
    multitask_ProcV[pid].context.ECX = 0;
    multitask_ProcV[pid].context.EDX = 0;
    multitask_ProcV[pid].context.ESI = 0;
    multitask_ProcV[pid].context.EDI = 0;
    multitask_ProcV[pid].context.EFLAGS = multitask_DefRegs.EFLAGS;
    multitask_ProcV[pid].context.EIP = (size_t)prog;
    multitask_ProcV[pid].context.CR3 = multitask_DefRegs.CR3;
    multitask_ProcV[pid].context.ESP = (size_t)multitask_ProcV[pid].stack + MULTITASK_STACKSIZE;
    multitask_ProcV[pid].freeze = false; multitask_ProcV[pid].active = true;
    multitask_ProcV[pid].file = false;
    return pid;
}

/**
 * @brief Function for execute program from file system
 * 
 * @param path Path of program file
 * 
 * @return Process ID of new process (-1 means failure)
 */
int exec(const char* path) {
    if (!multitask_InitLock || path == NULL || length(path) == 0) { return -1; }
    fs_Entry_t* stat = fs_stat(path); if (stat == NULL) { return -1; }
    void* data = fs_readFile(path); if (data == NULL) { return -1; }
    multitask_ProgELF32EH_t* eh = (multitask_ProgELF32EH_t*)data;
    if (*(uint32_t*)data != MULTITASK_PROGMAGIC) { return -1; }
    multitask_ProgELF32PH_t* phs = (multitask_ProgELF32PH_t*)(data + eh->e_phoff);
    uint32_t min_vaddr = 0xffffffff;
    uint32_t max_vaddr = 0;
    for (int i = 0; i < eh->e_phnum; i++) {
        multitask_ProgELF32PH_t *ph = &phs[i];
        if (ph->p_type != MULTITASK_PROGPTLOAD) continue;
        uint32_t seg_start = ph->p_vaddr;
        uint32_t seg_end   = ph->p_vaddr + ph->p_memsz;
        if (seg_start < min_vaddr) min_vaddr = seg_start;
        if (seg_end > max_vaddr)   max_vaddr = seg_end;
    }
    min_vaddr &= ~(MEMORY_BLKSIZE - 1);
    max_vaddr = (max_vaddr + MEMORY_BLKSIZE - 1) & ~(MEMORY_BLKSIZE - 1);
    void* base = malloc((size_t)(max_vaddr - min_vaddr)); if (base == NULL) { return -1; }
    for (int i = 0; i < eh->e_phnum; ++i) {
        multitask_ProgELF32PH_t* ph = &phs[i];
        if (ph->p_type != MULTITASK_PROGPTLOAD) { continue; }
        void* src = data + ph->p_offset;
        void* dst = (void*)((size_t)base + ph->p_vaddr - min_vaddr);
        ncopy(dst, src, ph->p_filesz);
        fill(dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        // printf("Segment %d: vaddr=0x%x, filesz=%d, memsz=%d, dst=0x%x\n",
        //     i, phs[i].p_vaddr, phs[i].p_filesz, phs[i].p_memsz,
        //     (unsigned int)(base + phs[i].p_vaddr));
    } void (*entry)() = (void (*)())((size_t)base + eh->e_entry);
    // INFO("0x%x", (size_t)base);
    int pid = spawn(path, entry); if (pid == -1) { free(base); return -1; }
    multitask_ProcV[pid].file = true;
    // INFO("0x%x", (size_t)entry);
    return pid;
}

/**
 * @brief Function for kill a process
 * 
 * @param pid Target process ID to kill
 * 
 * @return Operation status (-1 means failure)
 */
int kill(int pid) {
    if (!multitask_InitLock || !multitask_ProcV[pid].active ||
        pid <= 0 || pid >= MULTITASK_PROCLIMIT) { return -1; }
    free(multitask_ProcV[pid].stack);
    fill(multitask_ProcV[pid].name, 0, MULTITASK_NAMELIMIT);
    fill(&multitask_ProcV[pid].context, 0, sizeof(multitask_Ctx_t));
    multitask_ProcV[pid].active = false; return 0;
}

/**
 * @brief Function for switch to next process
 */
void yield() {
    if (!multitask_InitLock) { return; } multitask_InStream = true;
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_PROCLIMIT) { multitask_Focus = 0; }
    sentry(multitask_Focus);
    int next = 0; if (multitask_Focus == 0) {
        for (int i = 1; i < MULTITASK_PROCLIMIT; ++i) {
            if (multitask_ProcV[i].active && !multitask_ProcV[i].freeze) { next = i; break; }
        } if (next == 0) { PANIC("No processes to execute"); }
    } else {
        for (int i = multitask_Focus + 1; i < MULTITASK_PROCLIMIT; ++i) {
            if (multitask_ProcV[i].active && !multitask_ProcV[i].freeze) { next = i; break; }
        }
    } int old = multitask_Focus; if (old == next) { return; } multitask_Focus = next;
    void* oldctx = old ? &multitask_ProcV[old].context : &multitask_KernelProc.context;
    void* nextctx = next ? &multitask_ProcV[next].context : &multitask_KernelProc.context;
    multitask_swi(oldctx, nextctx);
}

/**
 * @brief Function for end current process
 */
void exit() {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_PROCLIMIT) { multitask_Focus = 0; }
    if (!multitask_InitLock || multitask_Focus == 0 || !multitask_InStream) { return; }
    kill(multitask_Focus); yield(); return;
}

/**
 * @brief Function for initialize multitasking system
 */
void multitask_init() {
    if (multitask_InitLock) { return; }
    multitask_ProcV = (multitask_Proc_t*)malloc(MULTITASK_PROCLIMIT * sizeof(multitask_Proc_t));
    if (multitask_ProcV == NULL) { PANIC("Out of memory"); }
    fill(multitask_ProcV, 0, MULTITASK_PROCLIMIT * sizeof(multitask_Proc_t));
    asm volatile("movl %%cr3, %%eax\t\n movl %%eax, %0":"=m"(multitask_DefRegs.CR3)::"%eax");
    asm volatile("pushfl\t\n movl (%%esp), %%eax\t\n movl %%eax, %0\t\n popfl":"=m"(multitask_DefRegs.EFLAGS)::"%eax");
    multitask_InitLock = true;
}