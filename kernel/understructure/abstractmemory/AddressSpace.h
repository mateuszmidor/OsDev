#ifndef ADDRESS_SPACE
#define ADDRESS_SPACE

#include "types.h"

namespace memory {

struct AddressSpace {
    u64     heap_low_limit;     // last address allocated for the heap, current program break
    u64     heap_high_limit;    // last address allocable for the heap
    u64     pml4_phys_addr;     // page table root physical address
};

} /* namespace memory */

#endif