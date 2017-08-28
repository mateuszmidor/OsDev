/**
 *   @file: Gdt.cpp
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#include "Gdt.h"

namespace hardware {

std::array<GdtEntry, Gate::GDT_MAX> Gdt::gdt;

/**
 * @brief   Replace current Global Descriptor Table with a new, ready for user-mode one
 */
void Gdt::reinstall_gdt() {
    setup_global_descriptor_table();
    install_global_descriptor_table();
}

void Gdt::setup_global_descriptor_table() {
    gdt[Gate::GDT_NULL] = make_entry(0, 0, 0);                     // NULL descriptor, obligatory in GDT at index 0
    gdt[Gate::GDT_USER_CODE] = make_entry(0x0, 0xFFFFFFFF, 3);     // EXECUTABLE(43) | ALWAYS1(44) | USERMODE(45,46) | PRESENT(47) | LONG_MODE(53)
    gdt[Gate::GDT_KERNEL_CODE] = make_entry(0x0, 0xFFFFFFFF, 0);   // EXECUTABLE(43) | ALWAYS1(44) | PRESENT(47) | LONG_MODE(53)
}

void Gdt::install_global_descriptor_table() {
    GdtSizeAddress gdt_size_address;
    gdt_size_address.size_minus_1 = sizeof(GdtEntry) * gdt.size() - 1;
    gdt_size_address.address = (u64) (gdt.data());

    asm (
        "lgdt %0 \n\t"
        "push %1 \n\t"
        "push $jump_to_new_gdt \n\t"
        "lretq \n\t"
        "jump_to_new_gdt: \n\t"
        : // no output
        : "m" (gdt_size_address), "g" (get_kernel_code_segment_selector())
    );
}

GdtEntry Gdt::make_entry(u32 base, u32 limit, u32 privilege) {
    GdtEntry g;

    g.limit_low = (limit & 0xFFFF);
    g.base_low = (base & 0xFFFFF);
    g.accessed = 0;
    g.read_write = 1;
    g.conforming_expand_down = 0;
    g.executable = 1;
    g.always_1 = 1;
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
} /* namespace hardware */
