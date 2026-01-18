.section .bss
tmp: .skip 4    # Reserve 4 bytes for restoring stack pointer (ESP)

.section .text
.global multitask_swi   # Set context switch as global function
multitask_swi:
    pusha   # Push all registers into stack (low to high: EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX. So pushes EAX first.)
    pushf   # Push flags (EFLAGS) register into stack
    mov %cr3, %eax  # Get CR3 register
    push %eax       # Push CR3 register
    # now, we have stack like that (low to high):
    # CR3, EFLAGS, EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX, return address, first parameter, second parameter
    mov 44(%esp), %eax  # Get first parameter from stack and set to EAX (old/current context to save)
    # Save some registers to first parameter (old/current context)
    mov %ebx, 4(%eax)   # EBX
    mov %ecx, 8(%eax)   # ECX
    mov %edx, 12(%eax)  # EDX
    mov %esi, 16(%eax)  # ESI
    mov %edi, 20(%eax)  # EDI
    # Move unsaved registers to saved registers
    mov 36(%esp), %ebx  # Load EAX into EBX
    mov 40(%esp), %ecx  # Load EIP into ECX
    mov 20(%esp), %edx  # Load ESP into EDX
    add $4, %edx        # Remove the return value by increasing ESP to 4
    mov 16(%esp), %esi  # Load EBP into ESI
    mov 4(%esp), %edi   # Load EFLAGS into EDI
    # Save unsaved registers to first parameter (old/current context)
    mov %ebx, 0(%eax)   # Save EAX
    mov %edx, 24(%eax)  # Save EIP
    mov %esi, 28(%eax)  # Save ESP
    mov %ecx, 32(%eax)  # Save EBP
    mov %edi, 36(%eax)  # Save EFLAGS
    pop %ebx            # Pop CR3 into EBX from stack
    mov %ebx, 40(%eax)  # Save CR3 to first parameter (old/current context)
    push %ebx           # Push CR3 contained EBX back into stack
    # Now, load the second parameter (new/next context)
    mov 48(%esp), %eax  # Load second parameter into EAX
    mov 24(%eax), %ebx  # Load ESP to EBX from second parameter
    sub $4, %ebx        # Decrease ESP contained EBX to 4 for set return value
    mov 32(%eax), %ecx  # Load EIP to ECX from second parameter
    mov %ecx, 0(%ebx)   # Load EIP contained ECX into top of ESP contained EBX
    mov %ebx, tmp       # Load ESP contained EBX into memorh
    # Load some registers to current registers from second parameter (stage 1)
    mov 40(%eax), %ebx  # Load CR3 into EBX
    mov 36(%eax), %ecx  # Load EFLAGS into ECX
    mov 0(%eax), %edx   # Load EAX into EDX
    mov 4(%eax), %esi   # Load EBX into ESI
    mov 8(%eax), %edi   # Load ECX into EDI
    # Load current loaded registers into stack (stage 1)
    mov %ebx, 0(%esp)   # Load CR3 contained EBX
    mov %ecx, 4(%esp)   # Load EFLAGS contained ECX
    mov %edx, 36(%esp)  # Load EAX contained EDX
    mov %esi, 24(%esp)  # Load EBX contained ESI
    mov %edi, 32(%esp)  # Load ECX contained EDI
    # Load some registers to current registers from second parameter (stage 2)
    mov 12(%eax), %ebx  # Load EDX into EBX
    mov 16(%eax), %ecx  # Load ESI into ECX
    mov 20(%eax), %edx  # Load EDI into EDX
    mov 28(%eax), %esi  # Load EBP into ESI
    # Load current loaded registers into stack (stage 2)
    mov %ebx, 28(%esp)  # Load EDX contained EBX
    mov %ecx, 12(%esp)  # Load ESI contained ECX
    mov %edx, 8(%esp)   # Load EDI contained EDX
    mov %esi, 16(%esp)  # Load EBP contained ESI
    # Restore everything
    pop %eax            # Load CR3 into EAX from stack
    mov %eax, %cr3      # Restore CR3 back
    popf                # Restore EFLAGS back
    popa                # Restore all registers
    mov tmp, %esp       # Restore ESP from memory
    ret                 # Return to next process
