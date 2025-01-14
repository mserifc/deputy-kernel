.set MAGIC, 0x1BADB002              # Multiboot Magic Number
.set FLAGS, (1<<0 | 1<<1)           # Multiboot Flags (Send Memory Info and Boot Dervice)
.set CHECKSUM, -(MAGIC + FLAGS)     # Multiboot Checksum for Verify Header

.section .multiboot     # Multiboot Header
    .long MAGIC         # Set Magic Number
    .long FLAGS         # Set Flags
    .long CHECKSUM      # Set Checksum

.section .text              # Code Section
    .extern kernel_init     # Include Kernel Initialize Function
    .global kernel_entry    # Make Kernel Entry Global Symbol

kernel_entry:                   # Entry of Kernel
    mov $kernel_stack, %esp     # Set Kernel Stack as Stack Pointer
    push %eax                   # Save EAX Register
    push %ebx                   # Save EBX Register
    call kernel_init            # Initialize Kernel

halt:           # Halt the CPU if Failed
    cli         # Disable Interrupts
    hlt         # Halt the CPU
    jmp halt    # If CPU escapes, bring to top of Halt Function

.section .bss               # BSS Section
    .space 2*1024*1024      # Reserve 2MB
kernel_stack:               # Set Kernel Stack Symbol
