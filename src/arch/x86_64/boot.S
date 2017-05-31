# ERROR CODE CONSTANTS
.equ ERROR_NO_KERNEL_LOADED, '0'
.equ ERROR_NO_CPUID, '1'
.equ ERROR_NO_LONGMODE, '2'

.section .data
hello: .asciz  "Hello in PhobOS!\0"

# kernel entry point
.global start
.section .text
start:
    # setup stack
    movl $stack_top, %esp

    # check if kernel was actually loaded by the bootloader
    call check_multiboot

    # check if CPUID is supported
    call check_cpuid

    # check if long mode is supported
    call check_long_mode

    # print `OK` to screen
    #movl $0x2f4b2f4f, 0xb8000
    mov $hello, %rsi
    call puts

    hlt

# put a string to the screen
# %rsi - null terminated string pointer
puts:
    # %rdi = vga text buffer address
    mov $0xb8000, %rdi

    .loop:
        # fetch one character
        movb (%rsi), %dl

        # check if \0
        cmp $0, %dl
        je .end_loop

        # copy characterto vga textbuffer
        movb %dl, (%rdi)
        movb $0x2f, 1(%rdi)

        # advance the source pointer and textbuffer pointer
        add $1, %rsi
        add $2, %rdi
        jmp .loop

    .end_loop:
        ret

# check if bootloader actually loaded our kernel
check_multiboot:
    cmp $0x36d76289, %eax   # check for magic number in eax
    jne .no_multiboot
    ret

.no_multiboot:
    movb $ERROR_NO_KERNEL_LOADED, %al
    jmp error

# check if CPUID is supported
check_cpuid:
    # Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    # in the FLAGS register. If we can flip it, CPUID is available.

    # Copy FLAGS in to EAX via stack
    pushf
    pop %rax

    # Copy to ECX as well for comparing later on
    movl %eax, %ecx

    # Flip the ID bit, 0x200000 = 1 << 21
    xorl $0x200000, %eax

    # Copy EAX to FLAGS via the stack
    push %rax
    popf

    # Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushf
    pop %rax

    # Restore FLAGS from the old version stored in ECX (i.e. flipping the
    # ID bit back if it was ever flipped).
    push %rcx
    popf

    # Compare EAX and ECX. If they are equal then that means the bit
    # wasn't flipped, and CPUID isn't supported.
    cmpl %eax, %ecx
    je .no_cpuid
    ret

.no_cpuid:
    movb $ERROR_NO_CPUID, %al
    jmp error


# check if longmode is supported
check_long_mode:
    # test if extended processor info in available
    movl $0x80000000, %eax  # implicit argument for cpuid
    cpuid                   # get highest supported argument
    cmpl $0x80000001, %eax  # it needs to be at least 0x80000001
    jb .no_long_mode        # if it's less, the CPU is too old for long mode

    # use extended info to test if long mode is available
    movl $0x80000001, %eax  # argument for extended processor info
    cpuid                   # returns various feature bits in ecx and edx
    test $0x20000000, %edx  # test if the LM-bit is set in the D-register, 0x20000000 = 2 << 29
    jz .no_long_mode        # If it's not set, there is no long mode
    ret

.no_long_mode:
    movb $ERROR_NO_LONGMODE, %al
    jmp error


# Prints `ERR: ` and the given error code to screen and hangs.
# parameter: error code (in ascii) in al
# 0 - kernel not loaded by bootloader
# 1 - no CPUID available
# 2 - no longmode available

error:
    movl $0x4f524f45, 0xb8000
    movl $0x4f3a4f52, 0xb8004
    movl $0x4f204f20, 0xb8008
    movb %al, 0xb800a
    hlt

# create stack memory
.section .bss
stack_bottom:
    .equ STACK_SIZE, 64
    .lcomm STACK_DATA, STACK_SIZE
stack_top:
