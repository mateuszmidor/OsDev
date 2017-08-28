/**
 *   @file: Gdt.cpp
 *
 *   @date: Aug 28, 2017
 * @author: Mateusz Midor
 */

#include <array>
#include "Gdt.h"

namespace hardware {

enum Gates : u16 {
    GDT_NULL = 0,
    GDT_KERNEL_CODE = 1,
    GDT_USER_CODE = 2,
    GDT_MAX
};
std::array<GdtEntry, Gates::GDT_MAX> gdt;

void Gdt::reinstall_gdt() {
    set_gate(Gates::GDT_NULL, 0, 0, 0);
    set_gate(Gates::GDT_KERNEL_CODE, 0x0, 0xFFFFFFFF, 0); // EXECUTABLE(43) | ALWAYS1(44) | PRESENT(47) | LONG_MODE(53)
    set_gate(Gates::GDT_USER_CODE, 0x0, 0xFFFFFFFF, 3); // EXECUTABLE(43) | ALWAYS1(44) | USERMODE(45,46) | PRESENT(47) | LONG_MODE(53)

    GdtSizeAddress gdt_size_address;
    gdt_size_address.size_minus_1 = sizeof(GdtEntry) * gdt.size() - 1;
    gdt_size_address.address = (u64)gdt.data();

    __asm__(
            "lgdt %0; \n\t"
            "push %1; \n\t"
            "push $reinstall2; \n\t"
            "lretq; \n\t"
            "reinstall2:; \n\t"

            : // no output
            : "m" (gdt_size_address), "g" (Gates::GDT_KERNEL_CODE * 8)
    );

}

void Gdt::set_gate(u32 gate_num, u32 base, u32 limit, u32 privilege) {
    auto& g = gdt[gate_num];

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
    g.gran = 0;
    g.base_high = (base >> 24) & 0xFF;
}
} /* namespace hardware */
