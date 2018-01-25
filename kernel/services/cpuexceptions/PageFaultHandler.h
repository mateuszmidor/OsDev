/**
 *   @file: PageFaultHandler.h
 *
 *   @date: Jun 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_CPUEXCEPTIONS_PAGEFAULTHANDLER_H_
#define SRC_CPUEXCEPTIONS_PAGEFAULTHANDLER_H_

#include "types.h"
#include "ExceptionHandler.h"

namespace cpuexceptions {

/**
 * @brief   Possible error code bitfields that come with PageFault exception
 * @see     http://wiki.osdev.org/Exceptions#Page_Fault, Error code
 */
enum PageFaultErrorCode {
    PRESENT     = 1,    // When set, the page fault was caused by a page-protection violation. When not set, it was caused by a non-present page.
    WRITE       = 2,    // When set, the page fault was caused by a page write. When not set, it was caused by a page read.
    USER        = 4,    // When set, the page fault was caused while CPL = 3. This does not necessarily mean that the page fault was a privilege violation.
    RESERVED    = 8,    // When set, the page fault was caused by reading a 1 in a reserved field.
    INSTR_FETCH = 16    // When set, the page fault was caused by an instruction fetch.
};

enum PageFaultActualReason {
    PAGE_NOT_PRESENT,
    READONLY_VIOLATION,
    PRIVILEGE_VIOLATION,
    RESERVED_WRITE_VIOLATION,
    INSTRUCTION_FETCH,
    STACK_OVERFLOW,
    UNKNOWN_PROTECTION_VIOLATION,
    INVALID_ADDRESS_SPACE
};


class PageFaultHandler: public ExceptionHandler {
    s16 handled_exception_no() override;
    hardware::CpuState* on_exception(hardware::CpuState* cpu_state) override;
    PageFaultActualReason page_fault_reason(u64* violated_page_addr, u64 error_code) const;
    bool alloc_page(size_t virtual_address, u64* page_virt_addr) const;
    size_t get_faulty_address();

    static const char* PF_REASON[];
};

} /* namespace cpuexceptions */

#endif /* SRC_CPUEXCEPTIONS_PAGEFAULTHANDLER_H_ */
