/**
    long_mode_start
    From here on we are in full 64bit long mode, but still using lower half addressing (C++ code will be run in higher half)
*/
.code64
.section .text
.extern KERNEL_VIRTUAL_BASE
.extern kmain
.global long_mode_start
long_mode_start:
    # load 0 into all data segment registers
    mov $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # print hello message to screen
    mov $HELLO_MSG, %rsi
    call print_string

    # enter c++ main routine using its absolute address - we start to execute higher half kernel here
    # %rdi holds multiboot2 information pointer so it's passed as 1st parameter according to SYSTEMV x64 ABI
    add KERNEL_VIRTUAL_BASE, %rsp   # make stack pointer a higher half pointer
    add KERNEL_VIRTUAL_BASE, %rdi   # make multiboot2 info pointer a higher half pointer
    movabs $kmain, %rax             # load absolute address of kmain
    call *%rax                      # jump to higher half kernel
    hlt                             # in case that kmain returns for some reason :)

/**
    print_string
    Put a string to the screen.
    %rsi - null terminated string address
*/
print_string:
    # %rdi = vga text buffer address, save it and restore upon return
    push %rdi

    mov $0xFFFFFFFF800b80A0, %rdi # second line in 80-cols 2-byte character vga text buffer

    .loop:
        # fetch one character
        movb (%rsi), %dl

        # check if \0
        cmp $0, %dl
        je .end_loop

        # copy character to vga textbuffer
        movb %dl, (%rdi)
        movb $0x2f, 1(%rdi) # 0x2f is white text on green background

        # advance the source pointer and textbuffer pointer
        add $1, %rsi
        add $2, %rdi
        jmp .loop

    .end_loop:
        pop %rdi
        ret

 .section .rodata
 HELLO_MSG: .asciz "Hello in PhobOS long mode 64bit!"
