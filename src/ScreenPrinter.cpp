/**
 *   @file: ScreenPrinter.cpp
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#include "ScreenPrinter.h"

void ScreenPrinter::set_fg_color(const Color value) {
    foreground = value;
}

void ScreenPrinter::set_bg_color(const Color value) {
    background = value;
}

void ScreenPrinter::clearscreen() {
    cursor_pos = 0;
    for (int i = 0; i < NUM_COLS * NUM_COLS; i++)
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
        vga[cursor_pos] = background << 12 | foreground << 8 | c;
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
    vga[cursor_pos] = Color::Black << 12 | Color::Black << 8 | ' ';
}
