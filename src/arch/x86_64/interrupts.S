.section .text


# exception handler without error code generator
.macro HandleException num
.global handle_exception_no_\num
handle_exception_no_\num:
    // clear error code as there is none
    movq $0, (error_code)
    movb $\num, (interrupt_number)
    jmp handle_interrupt
.endm


# exception handler with error code generator
.macro HandleExceptionWithErrorCode num
.global handle_exception_no_\num
handle_exception_no_\num:
    // error code comes at the top of the stack, so pop it into memory
    pop (error_code)
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


.extern _ZN16InterruptManager12on_interruptEhy
handle_interrupt:
    # save scratch registers
    push %rax
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11

    # handle interrupt/exception
    mov (error_code), %rsi
    mov (interrupt_number), %rdi
    call _ZN16InterruptManager12on_interruptEhy

    # restore scratch registers
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rax


    # divide by zero - fix the denominator
    mov $1, %rcx

    # return from interrupt handler
    iretq


# data section
.data
    error_code: .quad 0
    interrupt_number: .byte 0
