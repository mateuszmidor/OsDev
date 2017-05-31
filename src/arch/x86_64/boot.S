.global start

.section .text
start:
    # print `OK` to screen
    movl $0x2f4b2f4f, 0xb8000
    hlt
