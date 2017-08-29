/**
 *   @file: Gdt.h
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_HARDWARE_GDT_H_
#define SRC_HARDWARE_GDT_H_

#include <array>
#include "types.h"

namespace hardware {


enum Gate : u16 {
    GDT_NULL = 0,
    GDT_USER_CODE = 1,
    GDT_USER_DATA = 2,
    GDT_KERNEL_CODE = 3,
    GDT_KERNEL_DATA = 4,
    GDT_MAX
};

// A struct describing a Task State Segment.
struct TssEntry {
   u32 prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   u32 esp0;       // The stack pointer to load when we change to kernel mode.
   u32 ss0;        // The stack segment to load when we change to kernel mode.
   u32 esp1;       // Unused in long mode, from here down
   u32 ss1;
   u32 esp2;
   u32 ss2;
   u32 cr3;
   u32 eip;
   u32 eflags;
   u32 eax;
   u32 ecx;
   u32 edx;
   u32 ebx;
   u32 esp;
   u32 ebp;
   u32 esi;
   u32 edi;
   u32 es;
   u32 cs;
   u32 ss;
   u32 ds;
   u32 fs;
   u32 gs;
   u32 ldt;
   u16 trap;
   u16 iomap_base;  // sizeof(TssEntry) if no iomap used
} __attribute__((packed));

/**
 * Global Descriptor Table entry.
 * See: http://wiki.osdev.org/Global_Descriptor_Table
 */
struct GdtEntry {
    u32 limit_low               : 16;
    u32 base_low                : 24;

    // Access byte
    u32 accessed                : 1; // CPU will set it to 1 on accessing the GDT entry
    u32 read_write              : 1; // readable for code, writable for data
    u32 conforming_expand_down  : 1; // conforming for code, expand down for data
    u32 executable              : 1; // 0 for data segment, 1 for code segnemt
    u32 always_1                : 1; // should be 1 for everything but TSS and LDT
    u32 DPL                     : 2; // privilege level; 0 - kernel, 3 - user
    u32 present                 : 1; // segment is present, always set to 1

    u32 limit_high              : 4;

    // Flags
    u32 available               : 1; // set it to 1
    u32 long_mode               : 1; // for long mode set it to 1 and set big to 0
    u32 protected_mode          : 1; // for protected mode set it to 1 and long_mode to 0; 32bit opcodes for code, uint32_t stack for data
    u32 granularity             : 1; // 0 to interpret limit as bytes, 1 to interpret as 4k pages,

    u32 base_high               : 8;
} __attribute__((packed));

/**
 * This struct address is loaded with lgdt instruction
 */
struct GdtSizeAddress {
    u16 size_minus_1;
    u64 address;
} __attribute__((packed));

/**
 * @brief   Global Descriptor Table abstraction
 */
class Gdt {
public:
    void reinstall_gdt();
    static u64 get_null_segment_selector() { return gate_to_segment_selector(Gate::GDT_NULL); };
    static u64 get_user_code_segment_selector() { return gate_to_segment_selector(Gate::GDT_USER_CODE) | 0x03; }; // "| 0x03" to point out the ring 3 in the selector
    static u64 get_user_data_segment_selector() { return gate_to_segment_selector(Gate::GDT_USER_DATA) | 0x03; }; // "| 0x03" to point out the ring 3 in the selector
    static u64 get_kernel_code_segment_selector() { return gate_to_segment_selector(Gate::GDT_KERNEL_CODE); };
    static u64 get_kernel_data_segment_selector() { return gate_to_segment_selector(Gate::GDT_KERNEL_DATA); };

private:
    void setup_global_descriptor_table();
    void install_global_descriptor_table();
    GdtEntry make_entry(u32 base, u32 limit, u32 privilege, u32 is_executable);
    static u32 gate_to_segment_selector(Gate gate_num) { return gate_num * sizeof(GdtEntry); }

    static std::array<GdtEntry, Gate::GDT_MAX> gdt;
};

} /* namespace hardware */

#endif /* SRC_HARDWARE_GDT_H_ */
