.section .text

# exception handler without error code generator
.macro HandleException num
.global handle_exception_no_\num
handle_exception_no_\num:
    push $0 # no error_code here so just push 0 for CpuState::error_code struct sake
    movb $\num, (interrupt_number)
    jmp handle_interrupt
.endm


# exception handler with error code generator
.macro HandleExceptionWithErrorCode num
.global handle_exception_no_\num
handle_exception_no_\num:
    # error_code comes with exception at the top of the stack and lands just in CpuState::error_code field
    movb $\num, (interrupt_number)
    jmp handle_interrupt
.endm


# interrupt handler generator
.macro HandleInterrupt num
.global handle_interrupt_no_\num
handle_interrupt_no_\num:
    push $0 # no error_code here so just push 0 for CpuState::error_code struct sake
    movb $\num, (interrupt_number)
    jmp handle_interrupt
.endm


# generate handlers for exceptions
HandleException 0x00
HandleException 0x01
HandleException 0x02
HandleException 0x03
HandleException 0x04
HandleException 0x05
HandleException 0x06
HandleException 0x07
HandleExceptionWithErrorCode 0x08
HandleException 0x09
HandleExceptionWithErrorCode 0x0A
HandleExceptionWithErrorCode 0x0B
HandleExceptionWithErrorCode 0x0C
HandleExceptionWithErrorCode 0x0D
HandleExceptionWithErrorCode 0x0E
HandleException 0x0F
HandleException 0x10
HandleExceptionWithErrorCode 0x11
HandleException 0x12
HandleException 0x13
HandleException 0x14
HandleExceptionWithErrorCode 0x1E

# generate handlers for PIC interrupts, their numbers are shifted by Interrupts::IRQ_BASE so they dont mix with CPU exceptions
HandleInterrupt 0x20    # timer
HandleInterrupt 0x21    # keyboard
HandleInterrupt 0x2C    # mouse
HandleInterrupt 0x2E    # primary ata hdd
HandleInterrupt 0x2F    # secondary ata hdd

.extern _ZN16InterruptManager12on_interruptEhPN3cpu8CpuStateE
handle_interrupt:
    # save registers to CpuState struct, in the right order that reflects CpuState struct fields
    # CpuState::error_code is already pushed in HandleInterrupt/HandleException/HandleExceptionWithErrorCode
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rbp
    push %rdi
    push %rsi
    push %rdx
    push %rcx
    push %rbx
    push %rax

    # handle interrupt/exception
    mov (interrupt_number), %rdi    # interrupt number as first param
    mov %rsp, %rsi                  # stack pointer (holding CpuState struct) as second param
    call _ZN16InterruptManager12on_interruptEhPN3cpu8CpuStateE
    mov %rax, %rsp  # set stack pointer to the one returned from on_interrupt

    # restore scratch registers from CpuState struct
    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx
    pop %rsi
    pop %rdi
    pop %rbp
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15
    add $8, %rsp # pop CpuState::error_code from stack


.global ignore_interrupt
ignore_interrupt:
    # return from interrupt handler
    iretq


# data section
.section .data
    interrupt_number: .byte 0
