.global gdt_flush                       # Declare the gdt_flush function globally (so it can be called from other files)
.type gdt_flush, @function              # Specify that gdt_flush is a function

gdt_flush:
    movl 4(%esp), %eax                  # Get the address of the gdt_Pointer (passed on the stack) and load it into EAX Register
    lgdt (%eax)                         # Load the GDT (Global Descriptor Table) from the address in EAX into the CPU

    # Set up the segment registers (ds, es, fs, gs, ss) to point to the data segment
    movw $0x10, %ax                     # Load 0x10 into the lower 16 bits of AX. This is the data segment selector
    movw %ax, %ds                       # Move the value in AX (0x10) into the DS (Data Segment Register)
    movw %ax, %es                       # Move the value in AX (0x10) into the ES (Extra Segment Register)
    movw %ax, %fs                       # Move the value in AX (0x10) into the FS (FS Segment Register)
    movw %ax, %gs                       # Move the value in AX (0x10) into the GS (GS Segment Register)
    movw %ax, %ss                       # Move the value in AX (0x10) into the SS (Stack Segment Register)

    ljmp $0x08, $gdt_flush_segment      # Perform a far jump to a new code segment

gdt_flush_segment:
    ret                                 # Return from the function (this happens after jumping to the new segment)
