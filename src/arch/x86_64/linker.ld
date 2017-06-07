ENTRY(start)

SECTIONS {
    . = 1M;

    .boot : ALIGN(4K)
    {
        /* ensure that the multiboot header is at the beginning */
        *(.multiboot_header)
    }

    .text : ALIGN(4K)
    {
        *(.text .text.*)
    }
    
    .rodata : {
        *(.rodata .rodata.*)
    }
}