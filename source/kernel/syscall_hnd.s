.set SYSCALL_ENTCOUNT, 256  # System call entry count

.section .text
    .extern syscall_Table   # Include system call table pointer
    .global syscall_hnd     # Set system call handler as global
syscall_hnd:
    push %eax
    mov $0x10, %eax
    mov %eax, %ds
    pop %eax
    cmp $SYSCALL_ENTCOUNT, %eax # Compare entry count with EAX
    jae .syscall_hnd_err        # If greater, send error
    push %ebx                   # Save EBX content in stack
    movl $syscall_Table, %ebx   # Load system call table pointer into EBX
    # EAX contains the system call number. This operation shifts EAX content to 2 bits.
    # This makes it multiplied by 4. Because table has 4 byte sized addresses.
    shl $2, %eax
    add %ebx, %eax              # Now add the values
    pop %ebx                    # Restore EBX content back from stack
    cmpl $0, (%eax)             # Check the entry is empty or not
    je .syscall_hnd_err         # Send error if empty
    push %edi                   # Push 5th parameter of system call
    push %esi                   # Push 4th parameter of system call
    push %edx                   # Push 3rd parameter of system call
    push %ecx                   # Push 2nd parameter of system call
    push %ebx                   # Push 1st parameter of system call
    call *(%eax)                # Call the system call
    add $20, %esp               # Clean parameters from stack
    push %eax
    mov $0x23, %eax
    mov %eax, %ds
    pop %eax
    iret                        # Interrupt return
    .syscall_hnd_err:           # Error routine
        movl $-1, %eax          # Send -1 as return value
        iret                    # Interrupt return
