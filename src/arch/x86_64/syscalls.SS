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
    // %rax will contain syscall result value so no need to remember it

    // remember user stack pointer
    mov %rsp, (user_rsp)

    // switch to kernel stack
    mov $kernel_stack_top, %rsp

    // set 5 syscalls params as function args no. 2, 3, 4, 5, 6
    // see http://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64 and https://en.wikipedia.org/wiki/X86_calling_conventions "System V AMD64"
    mov %r8, %r9
    mov %r10, %r8
    mov %rdx, %rcx
    mov %rsi, %rdx
    mov %rdi, %rsi

    // set syscall number as function arg no. 1
    mov %rax, %rdi

    // call C++ syscall handler, result value comes back on %rax
    call on_syscall

    // restore user stack
    mov (user_rsp), %rsp

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
