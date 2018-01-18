/**
 *   @file: VgaPrinter.cpp
 *
 *   @date: Jan 12, 2018
 * @author: Mateusz Midor
 */

#include "String.h" // strlen
#include "VgaPrinter.h"

namespace utils {

VgaPrinter::VgaPrinter(drivers::VgaDriver& vga) : vga(vga) {
    vga.set_text_mode_90_30();
    vga.clear_screen();
    vga.set_cursor_visible(false);
}

void VgaPrinter::print(const char s[], middlespace::EgaColor c) {
    vga.print(current_col, current_row, s, c);
    current_col+= strlen(s);
}

void VgaPrinter::println(const char s[], middlespace::EgaColor c) {
    vga.print(current_col, current_row++, s, c);
    current_col = 0;
}

} /* namespace utils */
