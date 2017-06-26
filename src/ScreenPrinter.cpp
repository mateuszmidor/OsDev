/**
 *   @file: ScreenPrinter.cpp
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#include "ScreenPrinter.h"

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
