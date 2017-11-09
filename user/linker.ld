ENTRY(_start)

SECTIONS {
    . = 4M;
    
    .text ALIGN(4K) : {
        *(.text .text.*)
    }
    
    .init_array ALIGN(4K) : {
        start_ctors = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        end_ctors = .;
    }
    
    .fini_array ALIGN(4K) : {
        KEEP(*( .fini_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .fini_array.* )));
    } 
    
    .data ALIGN(4K) : {
        *(.data .data.*)
    }
    
    .rodata ALIGN(4K) : {
        *(.rodata .rodata.*)
    }
        
    .bss ALIGN(4K) : {
        *(.bss .bss.*)
    } 
    
    .others ALIGN(4K) : {
        *(*)
    } 
}