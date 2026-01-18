ENTRY(kernel_entry)             /* Set entry of the kernel */
OUTPUT_FORMAT(elf32-i386)       /* Build the kernel as 32-bit ELF format */
OUTPUT_ARCH(i386:i386)          /* For 32-bit x86 architecture machines */

SECTIONS
{
    . = 0x100000;               /* Base address of the kernel in physical memory */
    kernel_Base = .;            /* Kernel base address */

    .text :                     /* Text section */
    {
        *(.multiboot)           /* Multiboot information */
        *(.text*)               /* All text sections */
        *(.rodata)              /* Read-only data */
    }

    .data :                     /* Data section */
    {
        *(.data)                /* Data segment (variables) */
    }

    .bss :                      /* BSS section */
    {
        *(.bss)                 /* Uninitialized data */
        *(COMMON)               /* Common data (Uninitialized global variables) */
    }

    /DISCARD/ :                 /* Ignored sections */
    {
        *(.fini_array*)         /* Termination functions */
        *(.comment)             /* Comments */
    }

    . = ALIGN(4);               /* Memory alignment */
    kernel_Limit = .;           /* Kernel limit address */
}