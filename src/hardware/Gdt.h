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


enum Gates : u16 {
    GDT_NULL = 0,
    GDT_USER_CODE = 1,
    GDT_KERNEL_CODE = 2,
    GDT_MAX
};

// A struct describing a Task State Segment.
struct tss_entry_struct
{
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
   u32 es;         // The value to load into ES when we change to kernel mode.
   u32 cs;         // The value to load into CS when we change to kernel mode.
   u32 ss;         // The value to load into SS when we change to kernel mode.
   u32 ds;         // The value to load into DS when we change to kernel mode.
   u32 fs;         // The value to load into FS when we change to kernel mode.
   u32 gs;         // The value to load into GS when we change to kernel mode.
   u32 ldt;        // Unused...
   u16 trap;
   u16 iomap_base;
} __attribute__((packed));

/**
 * Global Descriptor Table entry.
 * See: http://wiki.osdev.org/Global_Descriptor_Table
 */
struct GdtEntry {
    u32 limit_low               : 16;
    u32 base_low                : 24;

    // Access byte
    u32 accessed                : 1;
    u32 read_write              : 1; // readable for code, writable for data
    u32 conforming_expand_down  : 1; // conforming for code, expand down for data
    u32 executable              : 1; // 1 for code, 0 for data
    u32 always_1                : 1; // should be 1 for everything but TSS and LDT
    u32 DPL                     : 2; // priviledge level; 0 - kernel, 3 - user
    u32 present                 : 1; // segment present, set to 1

    u32 limit_high              : 4;

    // Flags
    u32 available               : 1;
    u32 long_mode               : 1; // for long mode set it to 1 and set big to 0
    u32 protected_mode          : 1; // for protected mode set it to 1; 32bit opcodes for code, uint32_t stack for data
    u32 gran                    : 1; // 1 to interpret limit as 4k pages, 0 to interpret it as bytes
    u32 base_high               : 8;
} __attribute__((packed));

/**
 * This struct address is loade with lgdt instruction
 */
struct GdtSizeAddress {
    u16 size_minus_1;
    u64 address;
} __attribute__((packed));


class Gdt {
public:
    void reinstall_gdt();
    static u64 get_null_segment_selector() { return gate_to_segment_selector(Gates::GDT_NULL); };
    static u64 get_kernel_code_segment_selector() { return gate_to_segment_selector(Gates::GDT_KERNEL_CODE); };
    static u64 get_user_code_segment_selector() { return gate_to_segment_selector(Gates::GDT_USER_CODE); };

private:
    void setup_global_descriptor_table();
    void install_global_descriptor_table();
    GdtEntry make_entry(u32 base, u32 limit, u32 privilege);
    static u32 gate_to_segment_selector(u8 gate_num) { return gate_num * sizeof(GdtEntry); }

    static std::array<GdtEntry, Gates::GDT_MAX> gdt;
};

} /* namespace hardware */

#endif /* SRC_HARDWARE_GDT_H_ */
