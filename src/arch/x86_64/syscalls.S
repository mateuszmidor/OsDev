.section .text

.extern on_syscall
.extern kernel_stack_top
.global handle_syscall
handle_syscall:
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

    // remember user stack pointer
    mov %rsp, (user_rsp)

    // switch to kernel stack
    mov $kernel_stack_top, %rsp

    // set syscall number as first function argument
    mov %rax, %rdi
    call on_syscall

    // restore user stack
    mov (user_rsp), %rsp

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

    sysretq

.section .data
user_rsp:
    .quad 0
