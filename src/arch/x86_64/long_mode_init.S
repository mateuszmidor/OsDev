/**
    long_mode_start
    From here on we are in full 64bit long mode
*/
.code64
.section .text
.global long_mode_start
.extern kmain
long_mode_start:
    # load 0 into all data segment registers
    mov $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # print hello message to screen
    movabs $HELLO_MSG, %rsi
    call print_string


    # enter c++ generated main routine, dsi should still hold multiboot2 information pointer so it's passed as 1st parameter
    #call kmain
    hlt

/**
    print_string
    Put a string to the screen.
    %esi - null terminated string address
*/
print_string:
    # %edi = vga text buffer address, save it and restore upon return
    push %rdi
    mov $0xb80A0, %edi # second line in 80-cols 2-byte character vga text buffer

    .loop:
        # fetch one character
        movb (%esi), %dl

        # check if \0
        cmp $0, %dl
        je .end_loop

        # copy character to vga textbuffer
        movb %dl, (%edi)
        movb $0x2f, 1(%edi) # 0x2f is white text on green background

        # advance the source pointer and textbuffer pointer
        add $1, %esi
        add $2, %edi
        jmp .loop

    .end_loop:
        pop %rdi
        ret

 .section .rodata
 HELLO_MSG: .asciz "Hello in PhobOS long mode 64bit!"
