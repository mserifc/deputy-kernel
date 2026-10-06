#include "kernel.h"

#include "hw/protect.h"
#include "hw/interrupts.h"

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
        EIP, EFLAGS, CR3, CS, SS;
} PACKED multitask_Ctx_t;

// Structure of process
typedef struct {
    char name[MULTITASK_NAMELIMIT];     // Name of process
    multitask_Ctx_t context;            // Context structure
    void* stack; void* prog;            // Stack and program memory base pointer
    void* kstackbase; void* kstacktop;
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
//multitask_Proc_t multitask_KernelProc;

// Currently active process ID (0 at startup)
int multitask_Focus = 1;

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
    if (multitask_ProcV[pid].context.ESP <= (size_t)multitask_ProcV[pid].stack - kernel_UserlandBase) {
        reason = "stack explosion";
    } else if (multitask_ProcV[pid].context.ESP > ((size_t)multitask_ProcV[pid].stack + MULTITASK_STACKSIZE - kernel_UserlandBase)) {
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

// A switcher for jumping to first userland program
void multitask_ulswi(int pid) {
    /*INFO("0x%x\n0x%x\n0x%x", multitask_ProcV[pid].context.SS, multitask_ProcV[pid].context.CS, multitask_ProcV[pid].context.EIP);*/
    //INFO("%d: 0x%x", pid, multitask_ProcV[pid].context.EIP);
    extern void multitask_subulswi(uint32_t eip,uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss);
    multitask_subulswi(
        multitask_ProcV[pid].context.EIP,
        multitask_ProcV[pid].context.CS,
        multitask_ProcV[pid].context.EFLAGS,
        multitask_ProcV[pid].context.ESP,
        multitask_ProcV[pid].context.SS
    );
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
    multitask_ProcV[pid].stack = mapmem(MULTITASK_STACKSIZE, -1);
    multitask_ProcV[pid].kstackbase = mapmem(MULTITASK_STACKSIZE, -1);
    if (multitask_ProcV[pid].stack == NULL || multitask_ProcV[pid].kstackbase == NULL) {
        unmapmem(multitask_ProcV[pid].stack);
        unmapmem(multitask_ProcV[pid].kstackbase
        );
        return -1;
    } multitask_ProcV[pid].kstacktop = (void*)((size_t)multitask_ProcV[pid].kstackbase + MULTITASK_STACKSIZE);
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
    multitask_ProcV[pid].context.CS = multitask_DefRegs.CS;
    multitask_ProcV[pid].context.SS = multitask_DefRegs.SS;
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
    void* base = mapmem((size_t)(max_vaddr - min_vaddr), -1); if (base == NULL) { return -1; }
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
    int pid = spawn(path, entry); if (pid == -1) { unmapmem(base); return -1; }
    multitask_ProcV[pid].file = true;
    multitask_ProcV[pid].prog = base;
    multitask_ProcV[pid].context.CS = (3 * 8) | 3; // 0x1B
    multitask_ProcV[pid].context.SS = (4 * 8) | 3; // 0x23
    multitask_ProcV[pid].context.EIP-=kernel_UserlandBase;
    multitask_ProcV[pid].context.ESP-=kernel_UserlandBase;
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
    INFO();
    if (!multitask_InitLock ||
        pid <= 0 || pid >= MULTITASK_PROCLIMIT ||
        !multitask_ProcV[pid].active) { return -1; }
    free(multitask_ProcV[pid].stack);
    free(multitask_ProcV[pid].kstackbase);
    free(multitask_ProcV[pid].prog);
    unmapmem(multitask_ProcV[pid].stack);
    unmapmem(multitask_ProcV[pid].kstackbase);
    unmapmem(multitask_ProcV[pid].prog);
    fill(multitask_ProcV[pid].name, 0, MULTITASK_NAMELIMIT);
    fill(&multitask_ProcV[pid].context, 0, sizeof(multitask_Ctx_t));
    multitask_ProcV[pid].active = false; return 0;
}

typedef struct {
    uint32_t EDI;
    uint32_t ESI;
    uint32_t EBP;
    uint32_t ESP;
    uint32_t EBX;
    uint32_t EDX;
    uint32_t ECX;
    uint32_t EAX;
} PACKED generalpurpose_Registers_t;

/**
 * @brief Function for switch to next process
 */
NAKED void yield() { asm volatile ("int $0x9E\nret"); }
NAKED void sys_yield() {
    asm volatile (
        "pusha\n"
        "mov $0x10, %eax\n"
        "mov %eax, %ds\n"
        "lea 32(%esp), %eax\n"
        "push %eax\n"
        "lea 4(%esp), %eax\n"
        "push %eax\n"
        "call multitask_yield\n"
        "add $8, %esp\n"
        "mov $0x23, %eax\n"
        "mov %eax, %ds\n"
        "popa\n"
        "iret\n"
    );
}
void multitask_yield(generalpurpose_Registers_t* regs, interrupts_Frame_t* frame) {
    if (!multitask_InitLock) { return; } multitask_InStream = true;
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_PROCLIMIT) { multitask_Focus = 0; }
    sentry(multitask_Focus);
    int next = 0;
    for (int i = multitask_Focus + 1; i < MULTITASK_PROCLIMIT; ++i) {
        if (multitask_ProcV[i].active && !multitask_ProcV[i].freeze) { next = i; break; }
    } if (next == 0) {
        extern void core_worker(void); core_worker();
        for (int i = 1; i <= multitask_Focus; ++i) {
            if (multitask_ProcV[i].active && !multitask_ProcV[i].freeze) { next = i; break; }
        }
    } if (next == 0) { PANIC("No processes to execute"); }
    int old = multitask_Focus; if (old == next) { return; } multitask_Focus = next;
    multitask_Ctx_t* oldctx = &multitask_ProcV[old].context;
    multitask_Ctx_t* nextctx = &multitask_ProcV[next].context;
    protect_TSS.esp0 = (uint32_t)multitask_ProcV[next].kstacktop;
    //INFO("%s (0x%x) -> %s (0x%x)", multitask_ProcV[old].name, oldctx->CS, &multitask_ProcV[next].name, nextctx->CS);
    //INFO("0x%x\n", frame->cs);
    if ((frame->cs & 3) != 0) {
        oldctx->SS = frame->ss;
        oldctx->ESP = frame->sp;
        frame->ss = nextctx->SS;
        frame->sp = nextctx->ESP;
    }
    oldctx->EIP = frame->ip;
    frame->ip = nextctx->EIP;
    oldctx->CS = frame->cs;
    frame->cs = nextctx->CS;
    oldctx->EFLAGS = frame->flags;
    frame->flags = nextctx->EFLAGS;
    
    oldctx->EAX = regs->EAX;
    oldctx->EBX = regs->EBX;
    oldctx->ECX = regs->ECX;
    oldctx->EDX = regs->EDX;
    oldctx->ESI = regs->ESI;
    oldctx->EDI = regs->EDI;
    oldctx->EBP = regs->EBP;
    
    regs->EAX = nextctx->EAX;
    regs->EBX = nextctx->EBX;
    regs->ECX = nextctx->ECX;
    regs->EDX = nextctx->EDX;
    regs->ESI = nextctx->ESI;
    regs->EDI = nextctx->EDI;
    regs->EBP = nextctx->EBP;
    //multitask_swi((void*)oldctx, (void*)nextctx);
}

/**
 * @brief Function for end current process
 */
NAKED void exit() { asm volatile ("mov $1, %eax\nint $0x80\nret"); }
INTERRUPT GENERALREGSONLY void sys_exit(interrupts_Frame_t* frame) {
    if (multitask_Focus < 0 || multitask_Focus >= MULTITASK_PROCLIMIT) { multitask_Focus = 0; }
    if (!multitask_InitLock || multitask_Focus == 0 || !multitask_InStream) { return; }
    kill(multitask_Focus); for (int i = 0; i < MULTITASK_PROCLIMIT; ++i) { if(multitask_ProcV[i].active){INFO("%s", multitask_ProcV[i].name);}} //multitask_Focus = 0;
    int next = 0; for (int i = multitask_Focus + 1; i < MULTITASK_PROCLIMIT; ++i) {
        if (multitask_ProcV[i].active) {
            next = i; break;
        }
    } if (next == 0) {
        for (int i = 0; i < MULTITASK_PROCLIMIT; ++i) {
            if (multitask_ProcV[i].active) {
                next = i; break;
            }
        }
    } if (next == 0) { PANIC("No processes to execute"); }
    multitask_Focus = next;
    multitask_Ctx_t* ctx = &multitask_ProcV[next].context;
    frame->ip = ctx->EIP;
    frame->cs = ctx->CS;
    frame->flags = ctx->EFLAGS;
    if ((frame->cs & 3) != 0) {
        frame->ss = ctx->SS;
        frame->sp = ctx->ESP;
    }
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
    asm volatile ("movw %%cs, %0" : "=m" (multitask_DefRegs.CS));
    asm volatile ("movw %%ss, %0" : "=m" (multitask_DefRegs.SS));
    multitask_InitLock = true;
}
