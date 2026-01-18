%define MAGIC 0x1BADB002
%define FLAGS (1<<0 | 1<<1)
%define CHECKSUM -(MAGIC + FLAGS)

section .multiboot
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
    extern _kernel_main
    global loader

loader:
    mov esp, kernel_stack

    push eax
    push ebx

    call _kernel_main

_stop:
    cli
    hlt
    jmp _stop

section .bss
    resb 2*1024*1024
kernel_stack: