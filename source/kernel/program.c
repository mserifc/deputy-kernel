#include "kernel.h"

#define PROGRAM_MAGIC 0x464C457F // "\x7FELF"
#define PROGRAM_PTLOAD 1
#define PROGRAM_BASE 0x4000000

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
} PACKED program_ELF32EH_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} PACKED program_ELF32PH_t;

int program_exec(char* path) {
    if (path == NULL) { return -1; }
    fs_Entry_t* ent = fs_stat(path);
    if (ent == NULL) { ERR("Program path not exists: %s", path); return -1; }
    char* data = fs_readFile(path);
    if (data == NULL) { ERR("Unable to read program: %s", path); return -1; }
    program_ELF32EH_t* eh = (program_ELF32EH_t*)data;
    if (*(uint32_t*)data != PROGRAM_MAGIC) { ERR("Invalid magic: %s", path); return -1; }
    program_ELF32PH_t* phs = (program_ELF32PH_t*)(data + eh->e_phoff);
    for (int i = 0; i < eh->e_phnum; ++i) {
        program_ELF32PH_t* ph = &phs[i];
        if (ph->p_type != PROGRAM_PTLOAD) { continue; }
        void* src = data + ph->p_offset;
        void* dst = (void*)(PROGRAM_BASE + ph->p_vaddr);
        ncopy(dst, src, ph->p_filesz);
        fill(dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        printf("Segment %d: vaddr=0x%x, filesz=%d, memsz=%d, dst=0x%x\n",
            i, phs[i].p_vaddr, phs[i].p_filesz, phs[i].p_memsz,
            (unsigned int)(PROGRAM_BASE + phs[i].p_vaddr));
    } void (*entry)() = (void (*)())(PROGRAM_BASE + eh->e_entry); INFO("0x%x", (size_t)entry);
    // entry();
    spawn(path, entry); return 0;
}

/*

void load_user_elf(void* elf_buffer) {
    Elf32_Ehdr* eh = (Elf32_Ehdr*)elf_buffer;

    // ELF magic check
    if (!(eh->e_ident[0] == 0x7F && eh->e_ident[1] == 'E' &&
          eh->e_ident[2] == 'L' && eh->e_ident[3] == 'F')) {
        panic("Geçersiz ELF");
    }

    // Program headers
    Elf32_Phdr* ph = (Elf32_Phdr*)((char*)elf_buffer + eh->e_phoff);

    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;

        void* src = (char*)elf_buffer + ph[i].p_offset;
        void* dest = (void*)ph[i].p_vaddr;

        // paging ile bu adresi fiziksel belleğe map ediyorsan:
        map_pages_for_user(dest, ph[i].p_memsz); // örnek fonksiyon

        memcpy(dest, src, ph[i].p_filesz);
        memset(dest + ph[i].p_filesz, 0, ph[i].p_memsz - ph[i].p_filesz);
    }

    // Artık kullanıcı programı bellekte
    create_user_process((void*)eh->e_entry); // yeni process başlat
}


*/