ENTRY(start)

SECTIONS {
    . = 1M;

    .text : 
    {
        *(.multiboot_header)
        *(.text .text.*)
    }
    
    .data : {
        *(.data .data.*)
    }
    
    .rodata : {
        *(.rodata .rodata.*)
    }
    
    .bss : {
        *(.bss .bss.*)
    }    
}