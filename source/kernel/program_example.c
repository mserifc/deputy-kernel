#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define ELF_MAGIC 0x464C457F  // "\x7FELF"

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
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

typedef struct {
    uint32_t r_offset;
    uint32_t r_info;
} Elf32_Rel;

#define PT_LOAD    1
#define PT_DYNAMIC 2

#define R_386_RELATIVE 8
#define ELF32_R_TYPE(i) ((i)&0xff)

#define LOAD_BASE 0x1000000

void load_elf(uint8_t* elf_image) {
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)elf_image;

    // 1. Check ELF magic
    if (*(uint32_t*)elf_image != ELF_MAGIC) {
        // invalid ELF
        return;
    }

    // 2. Load program headers
    Elf32_Phdr* phdrs = (Elf32_Phdr*)(elf_image + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf32_Phdr* ph = &phdrs[i];
        if (ph->p_type != PT_LOAD) continue;

        void* src = elf_image + ph->p_offset;
        void* dst = (void*)(LOAD_BASE + ph->p_vaddr);

        memcpy(dst, src, ph->p_filesz);
        memset(dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }

    // 3. Handle relocations (only RELATIVE supported)
    // Relocations are often in .rel.dyn section, but here we just search manually
    Elf32_Rel* rel = NULL;
    size_t rel_count = 0;

    // HACK: Let's scan the file to find relocation section (example only)
    for (size_t i = 0; i < 0x10000; i += sizeof(Elf32_Rel)) {
        Elf32_Rel* maybe = (Elf32_Rel*)(elf_image + i);
        if (ELF32_R_TYPE(maybe->r_info) == R_386_RELATIVE) {
            rel = maybe;
            rel_count = 128; // assume 128 relocs max
            break;
        }
    }

    for (size_t i = 0; i < rel_count; i++) {
        Elf32_Rel* r = &rel[i];
        if (ELF32_R_TYPE(r->r_info) != R_386_RELATIVE) continue;

        uint32_t* reloc_addr = (uint32_t*)(LOAD_BASE + r->r_offset);
        *reloc_addr += LOAD_BASE;
    }

    // 4. Jump to entry point
    void (*entry_point)() = (void (*)())(LOAD_BASE + ehdr->e_entry);
    entry_point();
}