%define MAGIC 0x1BADB002
%define FLAGS (1<<0 | 1<<1)
%define CHECKSUM -(MAGIC + FLAGS)

section .multiboot
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
    extern _kernel_main
    global _kernel_entry

_kernel_entry:
    mov esp, _kernel_stack

    push eax
    push ebx
    
    cli

    jmp _kernel_main

_halt:
    cli
    hlt
    jmp _halt

section .bss
    resb 2*1024*1024
_kernel_stack: