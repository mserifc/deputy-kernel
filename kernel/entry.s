.set MAGIC, 0x1BADB002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

.section .text
    .extern _kernel_init
    .global _kernel_entry

_kernel_entry:
    mov $_kernel_stack, %esp
    push %eax
    push %ebx
    call _kernel_init

_halt:
    cli
    hlt
    jmp _halt

.section .bss
    .space 2*1024*1024
_kernel_stack:
