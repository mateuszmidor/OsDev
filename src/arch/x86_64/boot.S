# ERROR CODE CONSTANTS
.equ ERROR_NO_KERNEL_LOADED, '0'
.equ ERROR_NO_CPUID, '1'
.equ ERROR_NO_LONGMODE, '2'

/**
    Kernel Entry Point
    .code32 makes the gnu assembler generate 32bit instructions;
    this is necessary because GRUB throws us into 32bit proteced-mode.
    Once we switch to 64bit long mode, we can use .code64
*/
.code32
.section .text
.global start
start:
    # setup stack
    movl $stack_top, %esp

    # check if kernel was actually loaded by the bootloader
    call check_multiboot

    # check if CPUID is supported
    call check_cpuid

    # check if long mode is supported
    call check_long_mode

    call set_up_page_tables
    call enable_paging

    # print `OK` to screen
    #movl $0x2f4b2f4f, 0xb8000

    mov $hello, %esi
    call puts

    hlt

/**
    puts
    Put a string to the screen.
    %esi - null terminated string address
*/
puts:
    # %edi = vga text buffer address
    mov $0xb8000, %edi

    .loop:
        # fetch one character
        movb (%esi), %dl

        # check if \0
        cmp $0, %dl
        je .end_loop

        # copy characterto vga textbuffer
        movb %dl, (%edi)
        movb $0x2f, 1(%edi)

        # advance the source pointer and textbuffer pointer
        add $1, %esi
        add $2, %edi
        jmp .loop

    .end_loop:
        ret

/**
    check_multiboot
    Check if bootloader actually properly loaded our kernel
*/
check_multiboot:
    cmp $0x36d76289, %eax   # check for magic number in eax
    jne .no_multiboot
    ret

    .no_multiboot:
        movb $ERROR_NO_KERNEL_LOADED, %al
        jmp error

/**
    check_cpuid
    Check if CPUID instruction is supported
*/
check_cpuid:
    # Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    # in the FLAGS register. If we can flip it, CPUID is available.

    # Copy FLAGS in to EAX via stack
    pushf
    pop %eax

    # Copy to ECX as well for comparing later on
    movl %eax, %ecx

    # Flip the ID bit, 0x200000 = 1 << 21
    xorl $0x200000, %eax

    # Copy EAX to FLAGS via the stack
    push %eax
    popf

    # Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushf
    pop %eax

    # Restore FLAGS from the old veesion stored in ECX (i.e. flipping the
    # ID bit back if it was ever flipped).
    push %ecx
    popf

    # Compare EAX and ECX. If they are equal then that means the bit
    # wasn't flipped, and CPUID isn't supported.
    cmpl %eax, %ecx
    je .no_cpuid
    ret

    .no_cpuid:
        movb $ERROR_NO_CPUID, %al
        jmp error


/**
    check_long_mode
    Check if longmode is supported
*/
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

/**
    set_up_page_tables
    Setup page tables before going into long mode.
    Use huge pages (2MiB); so we only need to configure P4, P3, P2 page tables.
    P2 maps directly to a 2MiB page that is indexed with 21 remaining bits of the address, see:
    http://wiki.osdev.org/Page_Tables#2_MiB_pages_2
    This is identity mapping; virtual addresses map to the same physical addresses
*/
set_up_page_tables:
    # map first P4 entry to P3 table
    mov $p3_table, %eax
    or $0b11, %eax       # present + writable
    mov %eax, p4_table

    # map first P3 entry to P2 table
    mov $p2_table, %eax
    or $0b11, %eax       # present + writable
    mov %eax, p3_table

    # map each P2 entry to a huge 2MiB page
    mov $0, %ecx

    .map_p2_table:
        # map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
        mov $0x200000, %eax     # 2MiB
        mul %ecx                # start address of ecx-th page
        or $0b10000011, %eax    # present + writable + huge
        mov %eax, p2_table(, %ecx, 8) # map ecx-th entry; memory[p2_table + ecx * 8] = eax

        add $1, %ecx            # increase counter
        cmp $512, %ecx          # if counter == 512, the whole P2 table is mapped
        jne .map_p2_table       # else map the next entry

        ret

/**
    enable_paging
    Enable PAE, long mode and paging
*/
enable_paging:
    # load P4 to cr3 register (cpu uses this to access the P4 table)
    mov $p4_table, %eax
    mov %eax, %cr3

    # enable PAE-flag in cr4 (Physical Address Extension)
    mov %cr4, %eax
    or $32, %eax  # 1 << 5
    mov %eax, %cr4

    # set the long mode bit in the EFER MSR (model specific register)
    mov $0xC0000080, %ecx
    rdmsr
    or $256, %eax # 1 << 8
    wrmsr

    # enable paging in the cr0 register
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

    ret

/**
    error
    Prints `ERR: ` and the given error code to screen and hangs.
    %al - error code (in form of ascii character)
    '0' - kernel not loaded by bootloader
    '1' - no CPUID available
    '2' - no longmode available
*/
error:
    movl $0x4f524f45, 0xb8000
    movl $0x4f3a4f52, 0xb8004
    movl $0x4f204f20, 0xb8008
    movb %al, 0xb800a
    hlt


# zero-initialized data section
.section .bss

# make sure each page table fits into one memory page
.align 4096
# 4 level page tables for long mode, each has 512 entries of 8 byte pointer
.lcomm p4_table, 4096
.lcomm p3_table, 4096
.lcomm p2_table, 4096


# create stack memory
stack_bottom:
    .lcomm STACK_DATA, 64
stack_top:

.section .data
hello: .asciz "Hello in PhobOS!"

