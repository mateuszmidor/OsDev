ENTRY(start)

SECTIONS {
    . = 1M;

    .boot : ALIGN(0x1000)
    {
        /* ensure that the multiboot header is at the beginning */
        *(.multiboot)
    }

    .text : ALIGN(0x1000)
    {
        *(.text)
    }
}