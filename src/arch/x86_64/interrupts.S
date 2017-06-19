.section .text

.global handle_interrupt
handle_interrupt:
    push %rbp
    push %rdi
    push %rsi

    push %rdx
    push %rcx
    push %rbx
    push %rax

    # TODO: handle the interrupt here

    pop %rax
    pop %rbx
    pop %rcx
    pop %rdx

    pop %rsi
    pop %rdi
    pop %rbp

    # divide by zero - fix the denominator
    mov $1, %rbx

    iretq
