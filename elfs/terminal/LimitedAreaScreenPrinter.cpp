/**
 *   @file: LimitedAreaScreenPrinter.cpp
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#include "LimitedAreaScreenPrinter.h"

using namespace drivers;
namespace utils {


LimitedAreaScreenPrinter::LimitedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom) :
    left(left), top(top), right(right), bottom(bottom),
    cursor(left, top, right, bottom) {

    syscalls::vga_get_width_height(&vga_width, &vga_height);
    vga_buffer = new VgaCharacter[vga_width * vga_height];
    printable_area_width = right - left + 1;  // +1 because if right=1 and left=0 it makes 2 columns
    printable_area_height = bottom - top + 1; // +1 because if bottom=1 and top=0 it makes 2 rows
}

LimitedAreaScreenPrinter::~LimitedAreaScreenPrinter() {
    delete[] vga_buffer;
}

void LimitedAreaScreenPrinter::set_at(const Cursor& cursor, VgaCharacter c, bool autoflush) {
    return set_at(cursor.get_x(), cursor.get_y(), c, autoflush);
}

void LimitedAreaScreenPrinter::set_at(u16 x, u16 y, VgaCharacter c, bool autoflush) {
    vga_buffer[x + y * vga_width] = c;
    if (autoflush)
        flush_vga_buffer();
}

void LimitedAreaScreenPrinter::putc(const char c) {
    putc_into_vga(c);
}

void LimitedAreaScreenPrinter::putc_into_vga(const char c) {
    if (c == '\n')
        newline();
    else if (c == '\t')
        tab();
    else if (c == '\x08')
        backspace();
    else {
        set_at(cursor, VgaCharacter(c,  foreground, background));

        // advance cursor pos within destination area
        cursor++;
    }
}

void LimitedAreaScreenPrinter::newline() {
    cursor.newline();
}

void LimitedAreaScreenPrinter::tab() {
    putc(' ');
    putc(' ');
}

void LimitedAreaScreenPrinter::backspace() {
    // back cursor by 1 char
    cursor--;

    // clear character under cursor
    set_at(cursor, VgaCharacter { .character = ' ', .fg_color = foreground, .bg_color = background });
}

void LimitedAreaScreenPrinter::flush_vga_buffer() {
    syscalls::vga_flush_buffer((u16*)vga_buffer);
}
} /* namespace utils */
