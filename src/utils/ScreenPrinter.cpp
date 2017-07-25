/**
 *   @file: ScreenPrinter.cpp
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#include "ScreenPrinter.h"
#include "KernelLog.h"
#include "DriverManager.h"

using namespace drivers;
namespace utils {

BoundedAreaScreenPrinter::BoundedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom) :
    left(left), top(top), right(right), bottom(bottom), cursor_x(left), cursor_y(top) {

    printable_area_width = right - left + 1;  // +1 because if right=1 and left=0 it makes 2 columns
    printable_area_height = bottom - top + 1; // +1 because if bottom=1 and top=0 it makes 2 rows
}

void BoundedAreaScreenPrinter::set_cursor_pos(u8 x, u8 y) {
    vga->set_cursor_pos(x, y);
}

VgaCharacter& BoundedAreaScreenPrinter::at(u16 x, u16 y) {
    static VgaCharacter null;

    if (!vga) {
        auto& driver_manager = DriverManager::instance();
        vga = driver_manager.get_driver<VgaDriver>();
    }

    if (vga)
        return vga->at(x, y);
    else
        return null;
}

void BoundedAreaScreenPrinter::putc(const char c) {
    putc_into_vga(c);
}

void BoundedAreaScreenPrinter::putc_into_vga(const char c) {
    if (c == '\n')
        newline();
    else if (c == '\t')
        tab();
    else if (c == '\x08')
        backspace();
    else {
        at(cursor_x, cursor_y) = VgaCharacter { .character = c, .fg_color = foreground, .bg_color = background };

        // advance cursor pos within destination area
        if (cursor_x == right)
            newline();
        else
            cursor_x++;
    }

    set_cursor_pos(cursor_x, cursor_y);
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
    // back cursor by 1 char
    if (cursor_x > left)
        cursor_x--;
    else
        if (cursor_y > top)
            cursor_y--;

    // clear character under cursor
    at(cursor_x, cursor_y) = VgaCharacter { .character = ' ', .fg_color = foreground, .bg_color = background };
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

ScrollableScreenPrinter::ScrollableScreenPrinter() :
        BoundedAreaScreenPrinter(0, 0, 88, 29) {

    // add first, empty line to the buffer
    lines.push_back("");
}

void ScrollableScreenPrinter::scroll_up(u16 num_lines) {
    if (lines.size() <= printable_area_height)
        return;

    const u16 FIRST_LINE = 0;
    top_line = (top_line - num_lines <= FIRST_LINE) ? FIRST_LINE : top_line - num_lines;
    redraw();
}

void ScrollableScreenPrinter::scroll_down(u16 num_lines) {
    if (lines.size() <= printable_area_height)
        return;

    const s16 LAST_LINE = lines.size() - printable_area_height;
    top_line = (top_line + num_lines  >= LAST_LINE) ? LAST_LINE : top_line + num_lines;
    redraw();
}

void ScrollableScreenPrinter::scroll_to_begin() {
    if (lines.size() < printable_area_height)
        return;

    top_line = 0;
    redraw();
}

void ScrollableScreenPrinter::scroll_to_end() {
    if (lines.size() < printable_area_height)
        return;

    const s16 LAST_LINE = lines.size() - printable_area_height;
    top_line = (LAST_LINE <= 0) ? 0 : LAST_LINE;
    redraw();
}

void ScrollableScreenPrinter::putc(const char c) {
    // put on screen
    putc_into_vga(c);

    // put in text buffer
    putc_into_buffer(c);

    // scroll if writing below bottom of the screen
    scroll_down_if_needed();
}

void ScrollableScreenPrinter::putc_into_buffer(const char c) {
    if (c == '\n')
        lines.push_back("");
    else
    if (c == '\x08') {
        if (!lines.back().empty())
            lines.back().pop_back();
        else if (!lines.empty())
            lines.pop_back();
    } else
        lines.back() += c;

    // enforce text buffer "word" wrap
    if (lines.back().length() == printable_area_width)
        lines.push_back("");
}

void ScrollableScreenPrinter::scroll_down_if_needed() {
    // scroll if writing below bottom of the screen
    const u16 NUM_LINES = lines.size();
    if (NUM_LINES - top_line > printable_area_height)
        scroll_down(1);
}

/**
 * @brief   Redraw entire printing are. Used when scrolling the text
 */
void ScrollableScreenPrinter::redraw() {
    cursor_x = left;
    cursor_y = top;

    // calc number of buffer lines to  draw
    const u16 NUM_LINES = lines.size();
    u16 lines_to_draw = NUM_LINES - top_line;
    if (lines_to_draw > printable_area_height)
        lines_to_draw = printable_area_height;

    // draw the lines
    for (u16 i = 0; i < lines_to_draw; i++)
        put_line(lines[top_line + i]);

    // clear remaining lines
    for (u16 i = lines_to_draw; i < printable_area_height; i++)
        put_line("");

    draw_scroll_bar();
}

void ScrollableScreenPrinter::clear_screen() {
    cursor_x = left;
    cursor_y = top;

    for (u16 i = 0; i < printable_area_height; i++)
        put_line("");

    cursor_x = left;
    cursor_y = top;

    draw_scroll_bar();
}

void ScrollableScreenPrinter::draw_scroll_bar() {
    const u16 BAR_X = right + 1;

    // draw scroll bar dotted background
    for (u16 y = top; y <= bottom; y++)
        at(BAR_X, y) = VgaCharacter { .character = BG_CHAR, .fg_color = foreground, .bg_color = background };

    // draw actual scrollbar
    u16 bar_size = (printable_area_height > lines.size()) ? printable_area_height : printable_area_height * printable_area_height / lines.size();
    if (bar_size == 0)
        bar_size = 1;

    s16 max_top_line = lines.size() - printable_area_height;

    u16 move_space = printable_area_height - bar_size;
    u16 bar_y = top + move_space * top_line / max_top_line;

    for (u16 y = bar_y; y < bar_y + bar_size; y++)
        at(BAR_X, y) = VgaCharacter { .character = BG_SCROLLER, .fg_color = foreground, .bg_color = background };
}

void ScrollableScreenPrinter::put_line(const kstd::string& line) {
    u16 num_characters = line.size();

    // print the line
    for (u16 i = 0; i < num_characters; i++) {
        putc_into_vga(line[i]);
    }

    // clear remaining space of the printable area row
    for (u16 i = num_characters; i < printable_area_width; i++)
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

} // namespace utils
