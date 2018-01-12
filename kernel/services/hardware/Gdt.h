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

/**
 * @brief   Indexes for segment selectors in Global Descriptor Table, 64 bit long mode
 * @note    The order KERNEL CODE, KERNEL DATA, USER DATA, USER CODE is enforced by "syscall/sysret" x86-64 instructions
 */
enum Gate64 : u16 {
    GDT_NULL = 0,         // obligatory NULL  segment selector
    GDT_KERNEL_CODE = 1,  // segment selector for kernel-space code
    GDT_KERNEL_DATA = 2,  // segment selector for kernel-space data
    GDT_USER_DATA = 3,    // segment selector for user-space data
    GDT_USER_CODE = 4,    // segment selector for user-space code
    GDT_TSS = 5,          // segment selector for task state segment lower 8 bytes
    GDT_TSS_EXTENSION = 6,// segment selector for task state segment upper 8 bytes that hold upper 32 bits of base address of TSS struct
    GDT_MAX               // represents the segment selectors count
};

/**
 * @brief   A struct describing a Task State Segment for long mode. CPU loads stack address and io-privileges from TSS when interrupt comes.
 * @see     https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf, 6.7 TASK MANAGEMENT IN 64-BIT MODE
 */
struct TaskStateSegment64 {
    u32 reserved0;      // set to 0
    u64 rsp0;           // stack pointer for ring 0
    u64 rsp1;           // stack pointer for ring 1
    u64 rsp2;           // stack pointer for ring 2
    u32 reserved1;
    u32 reserved2;
    u64 ist1;           // interrupt stack table 1 and further
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u32 reserved3;
    u32 reserved4;
    u16 reserved5;
    u16 io_map_base;    // set to "io_map" offset within TaskStateSegment64
    u8  io_map[8192];   // bitmap for 65536 ports. "1" means access denied for rings other than 0
} __attribute__((packed));

/**
 * @brief   Global Descriptor Table entry
 * @see     http://wiki.osdev.org/Global_Descriptor_Table
 *          https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf, 3.4.5 Segment Descriptors
 */
struct GdtEntry {
    u32 limit_low               : 16;
    u32 base_low                : 24;

    // "Type" from intel manual
    u32 accessed                : 1; // CPU will set it to 1 on accessing the GDT entry
    u32 read_write              : 1; // readable for code segment, writable for data segment
    u32 conforming_expand_down  : 1; // conforming for code, expand down for data
    u32 executable              : 1; // 0 for data segment, 1 for code segment

    u32 code_or_data_segment    : 1; // 1 for code and data segments, 0 for TSS and LDT
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
 * @brief   In long mode, TSS GDT entry is extended to 128 bit, the upper 64 hold bits 32-63 of base address of the TSS struct
 * @see     https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pd, 6.2.3 TSS Descriptor in 64-bit mode
 */
struct GdtExtendedTssEntry {
    u32 base_upper_4_bytes     : 32; // bits 32-63 of base address
    u32 reserved0              : 8;  // set to 0
    u32 zero                   : 5;  // set to 0
    u32 reserved1              : 19; // set to 0
} __attribute__((packed));

/**
 * @brief This struct address is loaded with lgdt instruction
 * @see https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf, 3.5.1 Segment Descriptor Tables
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
    static u64 get_null_segment_selector() { return gate_to_segment_selector(Gate64::GDT_NULL); };
    static u64 get_kernel_code_segment_selector() { return gate_to_segment_selector(Gate64::GDT_KERNEL_CODE); };
    static u64 get_kernel_data_segment_selector() { return gate_to_segment_selector(Gate64::GDT_KERNEL_DATA); };      // GDT_NULL probably can be used here as there is no ds in kernel space long mode
    static u64 get_user_data_segment_selector() { return gate_to_segment_selector(Gate64::GDT_USER_DATA) | 0x03; };   // "| 0x03" to point out that this is ring 3 selector
    static u64 get_user_code_segment_selector() { return gate_to_segment_selector(Gate64::GDT_USER_CODE) | 0x03; };   // "| 0x03" to point out that this is ring 3 selector
    static u64 get_tss_segment_selector() { return gate_to_segment_selector(Gate64::GDT_TSS); };

private:
    void setup_task_state_segment();
    void setup_global_descriptor_table();
    void install_global_descriptor_table();
    void install_task_state_segment();
    GdtEntry make_null_entry();
    GdtEntry make_code_data_entry(u32 base, u32 limit, u32 privilege, u32 is_executable);
    GdtEntry make_tss_entry(u32 base, u32 limit);
    GdtEntry make_extended_tss_entry(u64 base);
    static u32 gate_to_segment_selector(Gate64 gate_num) { return gate_num * sizeof(GdtEntry); }


    static TaskStateSegment64 tss;
    static std::array<GdtEntry, Gate64::GDT_MAX> gdt;
};

} /* namespace hardware */

#endif /* SRC_HARDWARE_GDT_H_ */
