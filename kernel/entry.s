# Multiboot magic number
.set multiboot_magic, 0x1BADB002
# Multiboot flags (Request memory info and boot device)
.set multiboot_flags, (1<<0 | 1<<1)
# Multiboot checksum for verify header
.set multiboot_checksum, -(multiboot_magic + multiboot_flags)

.section .multiboot             # Multiboot section
    .long multiboot_magic       # Set magic
    .long multiboot_flags       # Set flags
    .long multiboot_checksum    # Set checksum

.section .text              # Code section
    .extern kernel_init     # Include kernel initialize function
    .global kernel_entry    # Define kernel entry as global symbol

kernel_entry:                   # Entry of kernel
    mov $kernel_stack, %esp     # Set kernel entry as stack pointer
    push %eax                   # Save EAX register to stack
    push %ebx                   # Save EBX register to stack
    call kernel_init            # Start the kernel initializer

kernel_halt:            # Halt the system if the initializer fails
    cli                 # Clear interrupt flag (Disable interrupts)
    hlt                 # Halt the CPU
    jmp kernel_halt     # If the CPU escapes, bring it to the top of the halt function again

.section .bss               # BSS section
    .space 2*1024*1024      # Reserve 2MB
kernel_stack:               # Set kernel stack symbol
