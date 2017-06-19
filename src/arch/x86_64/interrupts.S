.section .text


# exception handler generator macro
.macro HandleException num
.global handle_exception_no_\num
handle_exception_no_\num:
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
HandleException 0x08
HandleException 0x09
HandleException 0x0A
HandleException 0x0B
HandleException 0x0C
HandleException 0x0D
HandleException 0x0E
HandleException 0x0F
HandleException 0x10
HandleException 0x11
HandleException 0x12
HandleException 0x13


.extern on_interrupt
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
    mov (interrupt_number), %rdi
    call on_interrupt

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
    interrupt_number: .byte 0
