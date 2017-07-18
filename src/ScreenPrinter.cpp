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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

ScrollableScreenPrinter::ScrollableScreenPrinter(u16 vga_width, u16 vga_height) :
        BoundedAreaScreenPrinter(0, 0, vga_width-1, vga_height-1, vga_width, vga_height) {
    lines.push_back("");
}

void ScrollableScreenPrinter::scroll_up(u16 num_lines) {
    const u16 FIRST_LINE = 0;
    top_line = (top_line - num_lines <= FIRST_LINE) ? FIRST_LINE : top_line - num_lines;
    redraw();
}

void ScrollableScreenPrinter::scroll_down(u16 num_lines) {
    const u16 LAST_LINE = lines.size() - 1;
    top_line = (top_line + num_lines  >= LAST_LINE) ? LAST_LINE : top_line + num_lines;
    redraw();
}

void ScrollableScreenPrinter::putc(const char c) {
    if (c == '\n') {
        lines.push_back("");
        putc_into_vga('\n');
    }
    else {
        lines.back() += c;
        putc_into_vga(c);
    }

    if (lines.size() - top_line > vga_height) // eg 30 - 0 > 30
        scroll_to_end();
}

void ScrollableScreenPrinter::putc_into_vga(const char c) {
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
            newline();
        } else
            cursor_x++;
    }
}

void ScrollableScreenPrinter::scroll_to_end() {
    top_line = (lines.size() - vga_height <= 0) ? 0 : lines.size() - vga_height; // 31 - 30 ? 1
    redraw();
}

void ScrollableScreenPrinter::redraw() {
    const u16 MAX_LINES = bottom - top + 1;  // +1 because bottom 1 - top 0 gives 1, and they actual
    cursor_x = left;
    cursor_y = top;
    u16 lines_to_draw = (lines.size() - top_line > MAX_LINES) ? MAX_LINES : lines.size() - top_line;
    lines_to_draw = lines.size();
    if (lines_to_draw > 30)
        lines_to_draw = 30;


    for (u16 i = 0; i < lines_to_draw; i++)
        put_line(lines[top_line + i]);
}

void ScrollableScreenPrinter::put_line(const kstd::string& line) {
    // print line
    for (char c : line)
        putc_into_vga(c);

    // clear remaining part of the line
    for (u16 x = cursor_x; x <= right; x++)
        putc_into_vga(' ');
}


void ScrollableScreenPrinter::newline() {
    cursor_x = left;
    if (cursor_y < bottom)
        cursor_y++;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------



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
