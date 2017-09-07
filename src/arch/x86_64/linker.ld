ENTRY(start)

SECTIONS {
    . = 1M;

    .text : {
        *(.multiboot_header)
        *(.text .text.*)
    }
    
    .init_array : {
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        end_ctors = .;
    }
    
    .fini_array : {
        KEEP(*( .fini_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .fini_array.* )));
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