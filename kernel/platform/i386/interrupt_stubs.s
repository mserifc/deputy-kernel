# Extern functions
.extern interrupts_exceptionHandler     # Extern Exception Handler Function
.extern interrupts_IDTSetGate           # Extern IDT Set Gate Function

# Interrupt handlers for exceptions (0x00 to 0x1F)
.global interrupts_exception0x00    # 0
.global interrupts_exception0x01    # 1
.global interrupts_exception0x02    # 2
.global interrupts_exception0x03    # 3
.global interrupts_exception0x04    # 4
.global interrupts_exception0x05    # 5
.global interrupts_exception0x06    # 6
.global interrupts_exception0x07    # 7
.global interrupts_exception0x08    # 8
.global interrupts_exception0x09    # 9
.global interrupts_exception0x0A    # 10
.global interrupts_exception0x0B    # 11
.global interrupts_exception0x0C    # 12
.global interrupts_exception0x0D    # 13
.global interrupts_exception0x0E    # 14
.global interrupts_exception0x0F    # 15
.global interrupts_exception0x10    # 16
.global interrupts_exception0x11    # 17
.global interrupts_exception0x12    # 18
.global interrupts_exception0x13    # 19
.global interrupts_exception0x14    # 20
.global interrupts_exception0x15    # 21
.global interrupts_exception0x16    # 22
.global interrupts_exception0x17    # 23
.global interrupts_exception0x18    # 24
.global interrupts_exception0x19    # 25
.global interrupts_exception0x1A    # 26
.global interrupts_exception0x1B    # 27
.global interrupts_exception0x1C    # 28
.global interrupts_exception0x1D    # 29
.global interrupts_exception0x1E    # 30
.global interrupts_exception0x1F    # 31

# Exception handler initialize function
.global interrupts_exceptionInterruptsInit

# Interrupt handler functions for exception (0x00 to 0x1F)
interrupts_exception0x00:   # 0
    pushl $0x00
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x01:   # 1
    pushl $0x01
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x02:   # 2
    pushl $0x02
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x03:   # 3
    pushl $0x03
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x04:   # 4
    pushl $0x04
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x05:   # 5
    pushl $0x05
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x06:   # 6
    pushl $0x06
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x07:   # 7
    pushl $0x07
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x08:   # 8
    pushl $0x08
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x09:   # 9
    pushl $0x09
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x0A:   # 10
    pushl $0x0A
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x0B:   # 11
    pushl $0x0B
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x0C:   # 12
    pushl $0x0C
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x0D:   # 13
    pushl $0x0D
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x0E:   # 14
    pushl $0x0E
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x0F:   # 15
    pushl $0x0F
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x10:   # 16
    pushl $0x10
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x11:   # 17
    pushl $0x11
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x12:   # 18
    pushl $0x12
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x13:   # 19
    pushl $0x13
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x14:   # 20
    pushl $0x14
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x15:   # 21
    pushl $0x15
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x16:   # 22
    pushl $0x16
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x17:   # 23
    pushl $0x17
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x18:   # 24
    pushl $0x18
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x19:   # 25
    pushl $0x19
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x1A:   # 26
    pushl $0x1A
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x1B:   # 27
    pushl $0x1B
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x1C:   # 28
    pushl $0x1C
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x1D:   # 29
    pushl $0x1D
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x1E:   # 30
    pushl $0x1E
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret
interrupts_exception0x1F:   # 31
    pushl $0x1F
    call interrupts_exceptionHandler
    addl $0x04, %esp
    ret


// Function for initialize the interrupt handlers for exceptions
interrupts_exceptionInterruptsInit:
    lea interrupts_exception0x00, %eax  # 0
    pushl %eax
    pushl $0x00
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x01, %eax  # 1
    pushl %eax
    pushl $0x01
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x02, %eax  # 2
    pushl %eax
    pushl $0x02
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x03, %eax  # 3
    pushl %eax
    pushl $0x03
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x04, %eax  # 4
    pushl %eax
    pushl $0x04
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x05, %eax  # 5
    pushl %eax
    pushl $0x05
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x06, %eax  # 6
    pushl %eax
    pushl $0x06
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x07, %eax  # 7
    pushl %eax
    pushl $0x07
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x08, %eax  # 8
    pushl %eax
    pushl $0x08
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x09, %eax  # 9
    pushl %eax
    pushl $0x09
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x0A, %eax  # 10
    pushl %eax
    pushl $0x0A
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x0B, %eax  # 11
    pushl %eax
    pushl $0x0B
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x0C, %eax  # 12
    pushl %eax
    pushl $0x0C
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x0D, %eax  # 13
    pushl %eax
    pushl $0x0D
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x0E, %eax  # 14
    pushl %eax
    pushl $0x0E
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x0F, %eax  # 15
    pushl %eax
    pushl $0x0F
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x10, %eax  # 16
    pushl %eax
    pushl $0x10
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x11, %eax  # 17
    pushl %eax
    pushl $0x11
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x12, %eax  # 18
    pushl %eax
    pushl $0x12
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x13, %eax  # 19
    pushl %eax
    pushl $0x13
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x14, %eax  # 20
    pushl %eax
    pushl $0x14
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x15, %eax  # 21
    pushl %eax
    pushl $0x15
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x16, %eax  # 22
    pushl %eax
    pushl $0x16
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x17, %eax  # 23
    pushl %eax
    pushl $0x17
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x18, %eax  # 24
    pushl %eax
    pushl $0x18
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x19, %eax  # 25
    pushl %eax
    pushl $0x19
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x1A, %eax  # 26
    pushl %eax
    pushl $0x1A
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x1B, %eax  # 27
    pushl %eax
    pushl $0x1B
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x1C, %eax  # 28
    pushl %eax
    pushl $0x1C
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x1D, %eax  # 29
    pushl %eax
    pushl $0x1D
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x1E, %eax  # 30
    pushl %eax
    pushl $0x1E
    call interrupts_IDTSetGate
    addl $0x08, %esp
    lea interrupts_exception0x1F, %eax  # 31
    pushl %eax
    pushl $0x1F
    call interrupts_IDTSetGate
    addl $0x08, %esp
    ret
