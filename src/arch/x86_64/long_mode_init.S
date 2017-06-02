/**
    long_mode_start
    From here on we are in full 64bit long mode
*/
.code64
.section .text
.global long_mode_start
long_mode_start:
    # print `OKAY` to screen
    mov  $0x2f592f412f4b2f4f, %rax
    mov %rax, 0xb8000
    hlt
