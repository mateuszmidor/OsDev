/**
 *   @file: Gdt.cpp
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#include "kstd.h"
#include "Gdt.h"
/**
 * @brief   Statically allocated normal kernel stack (the top of it)
 * @see     boot.S
 */
extern u8 kernel_stack_top[];

/**
 * @brief   Kernel stack that is guaranteed to be valid; for handling Non-maskable Interrupts, Dobule faults and Machine Checks
 *          Used by the means of Interrupt Stack Tables
 * @see     https://software.intel.com/sites/default/files/managed/7c/f1/253668-sdm-vol-3a.pdf, 6.14.5 Interrupt Stack Table
 */
u8 kernel_stack_safe[1*4096];

namespace hardware {

TaskStateSegment64 Gdt::tss;
std::array<GdtEntry, Gate64::GDT_MAX> Gdt::gdt;

/**
 * @brief   Replace current Global Descriptor Table with a new, ready for user-mode, one
 */
void Gdt::reinstall_gdt() {
    setup_task_state_segment();
    setup_global_descriptor_table();
    install_global_descriptor_table();
    install_task_state_segment();
}

void Gdt::setup_task_state_segment() {
    // clear entire structure
    memset(&tss, 0, sizeof(tss));

    // set normal kernel stack pointer for ring0
    tss.rsp0 = (u64)kernel_stack_top;

    // set emergency kernel stack for handling emergency situation exceptions
    tss.ist1 = (u64)kernel_stack_safe + sizeof(kernel_stack_safe);


    // set io ports bitmap to deny any port access from rings other than ring0
    memset(&tss.io_map, 0xFF, sizeof(tss.io_map));
    tss.io_map_base = __builtin_offsetof(TaskStateSegment64, io_map);
}

void Gdt::setup_global_descriptor_table() {
    gdt[Gate64::GDT_NULL]         = make_null_entry();
    gdt[Gate64::GDT_KERNEL_CODE]  = make_code_data_entry(0x0, 0xFFFFFFFF, 0, 1);
    gdt[Gate64::GDT_KERNEL_DATA]  = make_code_data_entry(0x0, 0xFFFFFFFF, 0, 0);
    gdt[Gate64::GDT_USER_CODE]    = make_code_data_entry(0x0, 0xFFFFFFFF, 3, 1);
    gdt[Gate64::GDT_USER_DATA]    = make_code_data_entry(0x0, 0xFFFFFFFF, 3, 0);

    u64 base = (u64)&tss;
    u64 limit = base + sizeof(tss);
    gdt[Gate64::GDT_TSS]          = make_tss_entry(base & 0xFFFFFFFF, limit & 0xFFFFFFFF);
    gdt[Gate64::GDT_TSS_EXTENSION]= make_extended_tss_entry(base);
}

void Gdt::install_global_descriptor_table() {
    GdtSizeAddress gdt_size_address;
    gdt_size_address.address = (u64) (gdt.data());
    gdt_size_address.size_minus_1 = sizeof(GdtEntry) * gdt.size() - 1;

    asm volatile (
        "lgdt %0 \n\t"
        "push %1 \n\t"
        "push $jump_to_new_gdt \n\t"
        "lretq \n\t"
        "jump_to_new_gdt: \n\t"
        : // no output
        : "m" (gdt_size_address), "g" (get_kernel_code_segment_selector())
    );
}

void Gdt::install_task_state_segment() {
    u64 tss_selector = get_tss_segment_selector();
    asm volatile("ltrw %0"::"m"(tss_selector));
}

GdtEntry Gdt::make_null_entry() {
    GdtEntry g;
    memset(&g, 0, sizeof(g));
    return g;
}

GdtEntry Gdt::make_code_data_entry(u32 base, u32 limit, u32 privilege, u32 is_executable) {
    GdtEntry g;

    g.limit_low = (limit & 0xFFFF);
    g.base_low = (base & 0xFFFFFF);
    g.accessed = 0;
    g.read_write = 1;
    g.conforming_expand_down = 0;
    g.executable = is_executable;
    g.code_or_data_segment = 1;
    g.DPL = privilege;
    g.present = 1;
    g.limit_high = (limit >> 16) & 0x0F;
    g.available = 1;
    g.long_mode = 1;
    g.protected_mode = 0;
    g.granularity = 0;
    g.base_high = (base >> 24) & 0xFF;

    return g;
}

GdtEntry Gdt::make_tss_entry(u32 base, u32 limit) {
    GdtEntry g;

    g.limit_low = (limit & 0xFFFF);
    g.base_low = (base & 0xFFFFFF);

    g.accessed = 1;                 // always 1 for TSS
    g.read_write = 0;               // "Busy flag" for TSS
    g.conforming_expand_down = 0;   // always 0 for TSS
    g.executable = 1;               // always 1 for TSS

    g.code_or_data_segment = 0;     // 0 for system segment, eg. TSS
    g.DPL = 0;
    g.present = 1;

    g.limit_high = (limit >> 16) & 0x0F;
    g.available = 1;
    g.long_mode = 0;                // always 0 for TSS
    g.protected_mode = 0;
    g.granularity = 0;
    g.base_high = (base >> 24) & 0xFF;

    return g;
}

GdtEntry Gdt::make_extended_tss_entry(u64 base) {
    GdtExtendedTssEntry ext_tss;
    ext_tss.base_upper_4_bytes = base >> 32;
    ext_tss.reserved0 = 0;
    ext_tss.zero = 0;
    ext_tss.reserved1 = 0;

    return *((GdtEntry*)&ext_tss);
}
} /* namespace hardware */
