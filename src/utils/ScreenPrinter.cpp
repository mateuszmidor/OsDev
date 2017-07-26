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

void BoundedAreaScreenPrinter::set_cursor_visible(bool visible) {
    if (auto vga = get_vga())
        vga->set_cursor_visible(visible);
}

void BoundedAreaScreenPrinter::set_cursor_pos(u8 x, u8 y) {
    if (auto vga = get_vga())
        vga->set_cursor_pos(x, y);
}

std::shared_ptr<VgaDriver> BoundedAreaScreenPrinter::get_vga() {
    if (vga)
        return vga;

    auto& driver_manager = DriverManager::instance();
    vga = driver_manager.get_driver<VgaDriver>();
    return vga;
}

VgaCharacter& BoundedAreaScreenPrinter::at(u16 x, u16 y) {
    static VgaCharacter null;

    if (auto vga = get_vga())
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
        if (cursor_y > top) {
            cursor_y--;
            cursor_x = right;
        }

    // clear character under cursor
    at(cursor_x, cursor_y) = VgaCharacter { .character = ' ', .fg_color = foreground, .bg_color = background };
    set_cursor_pos(cursor_x, cursor_y);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

LineBuffer::LineBuffer() {
    // add first, empty line to the buffer
    lines.push_back("");
}

u32 LineBuffer::count() const {
    return lines.size();
}

void LineBuffer::push_back(const kstd::string& line) {
    lines.push_back(line);
}

void LineBuffer::putc(char c) {
    lines.back().push_back(c);
}

void LineBuffer::backspace() {
    if (lines.back().empty() && (lines.size() > 1)) // if the bottom line is empty and it is not the only one line in the buffer
        lines.pop_back();                           // remove the line

    if (!lines.back().empty())      // if the bottom line is not empty
        lines.back().pop_back();    // remove last character from the line
}

void LineBuffer:: newline() {
    lines.push_back("");
}

const kstd::string& LineBuffer::back() const {
    return lines.back();
}

const kstd::string& LineBuffer::operator[](u32 index) const {
    return lines[index];
}

ScrollableScreenPrinter::ScrollableScreenPrinter() :
        BoundedAreaScreenPrinter(0, 0, 88, 29) {
}

void ScrollableScreenPrinter::backspace() {
    BoundedAreaScreenPrinter::backspace();
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
