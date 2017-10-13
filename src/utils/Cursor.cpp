/**
 *   @file: Cursor.cpp
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#include "Cursor.h"
#include "DriverManager.h"

using namespace drivers;
namespace utils {

Cursor::Cursor(u16 left, u16 top, u16 right, u16 bottom) :
        left(left), top(top), right(right), bottom(bottom), cursor_x(left), cursor_y(top) {
}

u16 Cursor::get_x() const {
    return cursor_x;
}

u16 Cursor::get_y() const {
    return cursor_y;
}

void Cursor::set_x(u16 x) {
    cursor_x = x;
    update_vga_cursor();
}

void Cursor::set_y(u16 y) {
    cursor_y = y;
    update_vga_cursor();
}

void Cursor::operator++(int) {
    if (cursor_x == right)
        newline();
    else
        set_x(cursor_x + 1);
}

void Cursor::operator--(int) {
    if (cursor_x > left)
        set_x(cursor_x - 1);
    else
        if (cursor_y > top) {
            set_y(cursor_y - 1);
            set_x(right);
        }
}

void Cursor::newline() {
    set_x(left);
    if (cursor_y == bottom)
        set_y(top);
    else
        set_y(cursor_y + 1);
}

void Cursor::set_visible(bool visible) {
    if (auto vga = get_vga())
        vga->set_cursor_visible(visible);
}

void Cursor::update_vga_cursor() {
    if (auto vga = get_vga())
        vga->set_cursor_pos(cursor_x, cursor_y);
}

VgaDriver* Cursor::get_vga() {
    if (vga)
        return vga;

    auto& driver_manager = DriverManager::instance();
    vga = driver_manager.get_driver<VgaDriver>();
    return vga;
}

} /* namespace utils */
