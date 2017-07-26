/**
 *   @file: ScrollableScreenPrinter.cpp
 *
 *   @date: Jun 5, 2017
 * @author: Mateusz Midor
 */

#include "ScrollableScreenPrinter.h"


using namespace drivers;
namespace utils {

ScrollableScreenPrinter::ScrollableScreenPrinter(u16 left, u16 top, u16 right, u16 bottom) :
        LimitedAreaScreenPrinter(left, top, right - 1, bottom) { // right -1 for right scroll bar
}

void ScrollableScreenPrinter::newline() {
    LimitedAreaScreenPrinter::newline();
    lines.newline();
}

void ScrollableScreenPrinter::backspace() {
    LimitedAreaScreenPrinter::backspace();
    lines.backspace();
}

void ScrollableScreenPrinter::scroll_up(u16 num_lines) {
    if (lines.count() <= printable_area_height)
        return;

    const u16 FIRST_LINE = 0;
    top_line = (top_line - num_lines <= FIRST_LINE) ? FIRST_LINE : top_line - num_lines;
    redraw();
}

void ScrollableScreenPrinter::scroll_down(u16 num_lines) {
    if (lines.count() <= printable_area_height)
        return;

    const s16 LAST_LINE = lines.count() - printable_area_height;
    top_line = (top_line + num_lines  >= LAST_LINE) ? LAST_LINE : top_line + num_lines;
    redraw();
}

void ScrollableScreenPrinter::scroll_to_begin() {
    if (lines.count() <= printable_area_height)
        return;

    top_line = 0;
    redraw();
}

void ScrollableScreenPrinter::scroll_to_end() {
    if (lines.count() <= printable_area_height)
        return;

    const s16 LAST_LINE = lines.count() - printable_area_height;
    top_line = LAST_LINE;
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
        lines.newline();
    else
    if (c == '\x08') {
        lines.backspace();
    } else
        lines.putc(c);

    // enforce text buffer "word" wrap
    if (lines.back().length() == printable_area_width)
        lines.newline();
}

void ScrollableScreenPrinter::scroll_down_if_needed() {
    // scroll if writing below bottom of the screen
    if (!is_edit_line_visible())
        scroll_down(1);
}

bool ScrollableScreenPrinter::is_edit_line_visible() {
    return (s16)lines.count() - printable_area_height <= top_line;
}

/**
 * @brief   Redraw entire printing are. Used when scrolling the text
 */
void ScrollableScreenPrinter::redraw() {
    // calc number of buffer lines to  draw
    u16 lines_to_draw = lines.count() - top_line;
    if (lines_to_draw > printable_area_height)
        lines_to_draw = printable_area_height;

    // draw the lines
    for (u16 y = 0; y < lines_to_draw; y++) 
        put_line_and_clear_remaining_space_at(y, lines[top_line + y]);
   

    // clear remaining lines
    for (u16 y = lines_to_draw; y < printable_area_height; y++) 
        put_line_and_clear_remaining_space_at(y, "");

    draw_scroll_bar();

    set_cursor_visible(is_edit_line_visible());

    cursor_x = left + lines.back().length();
    cursor_y = top + lines_to_draw -1;
    set_cursor_pos(cursor_x, cursor_y);
}

void ScrollableScreenPrinter::clear_screen() {
    redraw();
}

void ScrollableScreenPrinter::draw_scroll_bar() {
    const u16 BAR_X = right + 1;

    // draw scroll bar dotted background
    for (u16 y = top; y <= bottom; y++)
        at(BAR_X, y) = VgaCharacter { .character = BG_CHAR, .fg_color = foreground, .bg_color = background };

    // draw actual scrollbar
    u16 bar_size = (printable_area_height > lines.count()) ? printable_area_height : printable_area_height * printable_area_height / lines.count();
    if (bar_size == 0)
        bar_size = 1;

    s16 max_top_line = lines.count() - printable_area_height;

    u16 move_space = printable_area_height - bar_size;
    u16 bar_y = top + move_space * top_line / max_top_line;

    for (u16 y = bar_y; y < bar_y + bar_size; y++)
        at(BAR_X, y) = VgaCharacter { .character = BG_SCROLLER, .fg_color = foreground, .bg_color = background };
}

void ScrollableScreenPrinter::put_line_and_clear_remaining_space_at(u8 y, const kstd::string& line) {
    u16 num_characters = line.size();

    // print the line
    for (u16 i = 0; i < num_characters; i++) {
        at(i, y) = VgaCharacter { .character = line[i], .fg_color = foreground, .bg_color = background };
    }

    // clear remaining space of the row
    for (u16 i = num_characters; i < printable_area_width; i++)
        at(i, y) = VgaCharacter { .character = ' ', .fg_color = foreground, .bg_color = background };
}


} // namespace utils
