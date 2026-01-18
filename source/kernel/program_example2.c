#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>

#define LOAD_ADDR 0x08000000  // Programı buraya yükleyeceğiz
#define STACK_SIZE 0x10000    // 64KB stack

void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Kullanım: %s <elf>\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open");

    struct stat st;
    if (fstat(fd, &st) < 0) die("fstat");

    void *elf_data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (elf_data == MAP_FAILED) die("mmap");

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf_data;

    // ELF geçerlilik kontrolü
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
        ehdr->e_ident[EI_DATA] != ELFDATA2LSB ||
        ehdr->e_type != ET_DYN ||
        ehdr->e_machine != EM_386) {
        fprintf(stderr, "Geçersiz veya desteklenmeyen ELF dosyası.\n");
        exit(1);
    }

    Elf32_Phdr *phdrs = (Elf32_Phdr *)((char *)elf_data + ehdr->e_phoff);

    // 1. Segmentleri belleğe yükle
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if (phdrs[i].p_type != PT_LOAD) continue;

        uint32_t seg_vaddr = LOAD_ADDR + phdrs[i].p_vaddr;
        uint32_t memsz = phdrs[i].p_memsz;
        uint32_t filesz = phdrs[i].p_filesz;
        uint32_t offset = phdrs[i].p_offset;

        void *dest = mmap((void *)seg_vaddr, memsz,
                          PROT_READ | PROT_WRITE | PROT_EXEC,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                          -1, 0);
        if (dest == MAP_FAILED) die("mmap segment");

        memcpy(dest, (char *)elf_data + offset, filesz);
        if (memsz > filesz) {
            memset((char *)dest + filesz, 0, memsz - filesz);
        }

        printf("Segment yüklendi: 0x%x - 0x%x\n", seg_vaddr, seg_vaddr + memsz);
    }

    // 2. Relocation işle (sadece R_386_RELATIVE destekliyoruz)
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if (phdrs[i].p_type == PT_DYNAMIC) {
            Elf32_Dyn *dyn = (Elf32_Dyn *)(LOAD_ADDR + phdrs[i].p_vaddr);

            Elf32_Rel *rel = NULL;
            size_t relsz = 0;

            for (; dyn->d_tag != DT_NULL; ++dyn) {
                if (dyn->d_tag == DT_REL) {
                    rel = (Elf32_Rel *)(LOAD_ADDR + dyn->d_un.d_ptr);
                } else if (dyn->d_tag == DT_RELSZ) {
                    relsz = dyn->d_un.d_val;
                }
            }

            if (rel && relsz) {
                size_t count = relsz / sizeof(Elf32_Rel);
                for (size_t j = 0; j < count; ++j) {
                    uint32_t *patch_addr = (uint32_t *)(LOAD_ADDR + rel[j].r_offset);
                    uint32_t type = ELF32_R_TYPE(rel[j].r_info);
                    if (type == R_386_RELATIVE) {
                        *patch_addr += LOAD_ADDR;
                    } else {
                        fprintf(stderr, "Desteklenmeyen relocation türü: %d\n", type);
                        exit(1);
                    }
                }
                printf("Relocation (%zu adet) uygulandı.\n", count);
            }
        }
    }

    // 3. Stack hazırla
    void *stack = mmap((void *)(LOAD_ADDR + 0x01000000 - STACK_SIZE),
                       STACK_SIZE,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                       -1, 0);
    if (stack == MAP_FAILED) die("mmap stack");

    uint32_t stack_top = (uint32_t)stack + STACK_SIZE;

    // 4. Entry point'e atla
    void (*entry)(void) = (void (*)(void))(LOAD_ADDR + ehdr->e_entry);

    printf("Program başlatılıyor: 0x%x\n", (uint32_t)entry);

    asm volatile (
        "movl %0, %%esp\n"
        "xor %%ebp, %%ebp\n"
        "jmp *%1\n"
        :
        : "r"(stack_top), "r"(entry)
        : "memory"
    );

    return 0;
}