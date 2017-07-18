/**
 *   @file: ScreenPrinter.cpp
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#include "ScreenPrinter.h"

BoundedAreaScreenPrinter::BoundedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom, u16 vga_width, u16 vga_height) :
    left(left), top(top), right(right), bottom(bottom), vga_width(vga_width), vga_height(vga_height), cursor_x(left), cursor_y(top) {
}

void BoundedAreaScreenPrinter::putc(const char c) {
    if (c == '\n')
        newline();
    else if (c == '\t')
        tab();
    else if (c == '\x08')
        backspace();
    else {
        u32 cursor_pos = cursor_y * vga_width + cursor_x;
        vga[cursor_pos] = VgaCharacter { .character = c, .fg_color = foreground, .bg_color = background };

        // advance cursor pos within destination area with horizontal and vertical wrapping
        if (cursor_x == right) {
            cursor_x = left;
            if (cursor_y == bottom)
                cursor_y = top;
            else
                cursor_y++;
        } else
            cursor_x++;
    }
}

void BoundedAreaScreenPrinter::newline() {
    cursor_x = left;
    if (cursor_y == bottom)
        cursor_y = top;
    else
        cursor_y++;
}

void BoundedAreaScreenPrinter::tab() {
    putc(' ');
    putc(' ');
}

void BoundedAreaScreenPrinter::backspace() {
}


ScreenPrinter ScreenPrinter::_instance;

ScreenPrinter& ScreenPrinter::instance() {
    return _instance;
}

VgaCharacter& ScreenPrinter::at(u16 x, u16 y) {
    if ((x > NUM_COLS) || (y > NUM_ROWS))
        return vga[0];

    return vga[y * NUM_COLS + x];
}

void ScreenPrinter::move_to(u16 x, u16 y) {
    if ((x > NUM_COLS) || (y > NUM_ROWS))
        return;

    cursor_pos = y * NUM_COLS + x;
}

void ScreenPrinter::swap_fg_bg_at(u16 x, u16 y) {
    VgaCharacter c = at(x, y);
    at(x, y) = VgaCharacter { .character = c.character, .fg_color = c.bg_color, .bg_color = c.fg_color };
}

void ScreenPrinter::set_fg_color(const drivers::EgaColor value) {
    foreground = value;
}

void ScreenPrinter::set_bg_color(const drivers::EgaColor value) {
    background = value;
}

void ScreenPrinter::clearscreen() {
    cursor_pos = 0;
    for (u16 i = 0; i < NUM_COLS * NUM_COLS; i++)
        putc(' ');
}

void ScreenPrinter::putc(const char c) {
    if (c == '\n')
        newline();
    else if (c == '\t')
        tab();
    else if (c == '\x08')
        backspace();
    else {
        vga[cursor_pos] = VgaCharacter { .character = c, .fg_color = foreground, .bg_color = background };
        cursor_pos = (cursor_pos + 1 ) % (NUM_COLS * NUM_ROWS);
    }
}

void ScreenPrinter::newline() {
    cursor_pos = ((cursor_pos / NUM_COLS * NUM_COLS) + NUM_COLS) % (NUM_COLS * NUM_ROWS);
}

void ScreenPrinter::tab() {
    putc(' ');
    putc(' ');
}

void ScreenPrinter::backspace() {
    cursor_pos = (cursor_pos + NUM_COLS * NUM_ROWS - 1) % (NUM_COLS * NUM_ROWS);
    vga[cursor_pos] = VgaCharacter { .character = ' ', .fg_color = drivers::EgaColor::Black, .bg_color = drivers::EgaColor::Black };
}
