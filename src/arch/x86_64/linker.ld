ENTRY(start)

SECTIONS {
    . = 1M;

    .text : 
    {
        *(.multiboot_header)
        *(.text .text.*)
    }
    
    .data : {
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        end_ctors = .;
         
        *(.data .data.*)
    }
    
    .rodata : {
        *(.rodata .rodata.*)
    }
    
    .bss : {
        *(.bss .bss.*)
    }    
}