# Multiboot magic number
.set multiboot_Magic, 0x1BADB002
# Multiboot flags (Request page align, memory info and video mode)
.set multiboot_Flags, (1<<0 | 1<<1 | 1<<2)
# Multiboot checksum for verify header
.set multiboot_Checksum, -(multiboot_Magic + multiboot_Flags)

.section .multiboot             # Multiboot section
    .long multiboot_Magic       # Set magic
    .long multiboot_Flags       # Set flags
    .long multiboot_Checksum    # Set checksum
    .long 0,0,0,0,0             # aout kludge (unused)
    .long 0                     # Request linear framebuffer
    .long 0                     # Preferred graphic width (0 means default)
    .long 0                     # Preferred graphic height (0 means default)
    .long 32                    # Preferred pixel depth

.section .text              # Code section
    .extern kernel_init     # Include kernel initialize function
    .global kernel_entry    # Define kernel entry as global symbol

kernel_entry:                   # Entry of kernel
    mov $kernel_Stack, %esp     # Set kernel entry as stack pointer
    push %eax                   # Save EAX register to stack
    push %ebx                   # Save EBX register to stack
    call kernel_init            # Start the kernel initializer

kernel_halt:            # Halt the system if the initializer fails
    cli                 # Clear interrupt flag (Disable interrupts)
    hlt                 # Halt the CPU
    jmp kernel_halt     # If the CPU escapes, bring it to the top of the halt function again

.section .bss               # BSS section
    .space 64*1024          # Reserve 64KB for kernel stack
kernel_Stack:               # Set kernel stack symbol here
