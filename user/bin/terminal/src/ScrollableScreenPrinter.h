/**
 *   @file: ScrollableScreenPrinter.h
 *
 *   @date: Jun 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_SCREENPRINTER_H_
#define SRC_SCREENPRINTER_H_

#include "Vga.h"
#include "Cursor.h"
#include "LineBuffer.h"
#include "StringUtils.h"

namespace terminal {


class ScrollableScreenPrinter {
public:
    ScrollableScreenPrinter(u16 left, u16 top, u16 right, u16 bottom);
    virtual ~ScrollableScreenPrinter();

    void scroll_up(u16 lines);
    void scroll_down(u16 lines);
    void scroll_to_begin();
    void scroll_to_end();
    void clear_screen();

    /**
     * @name    format
     * @example format("CPU: %", cpu_vendor_cstr);
     */
    template<typename ... Args>
    void format(const ustd::string& fmt, Args ... args) {
        const ustd::string& str = ustd::StringUtils::format(fmt, args...);
        for (const char& c : str)
            putc(c);
        redraw();
    }

    void format(const char c) {
        putc(c);
        redraw();
    }

private:
    void newline();
    void tab();
    void backspace();
    void set_at(const Cursor& cursor,  middlespace::VgaCharacter c);
    void set_at(u16 x, u16 y, middlespace::VgaCharacter c);
    void putc(const char c);
    void redraw();
    void draw_text(u16 lines_to_draw);
    void draw_scroll_bar();
    void put_line_and_clear_remaining_space_at(u16 y, const ustd::string& line);
    bool is_edit_line_visible();
    void flush_vga_buffer();

    const char BG_CHAR      = 176;
    const char BG_SCROLLER  = 219;
    const middlespace::EgaColor foreground = middlespace::EgaColor::White;
    const middlespace::EgaColor background = middlespace::EgaColor::Black;

    u16 vga_width           = 0;    // screen dimmensions in characters eg 90x30
    u16 vga_height          = 0;

    u16 left, top, right, bottom;   // printable area bounding box

    u16 printable_area_width;       // printable area dimension
    u16 printable_area_height;

    middlespace::VgaCharacter* vga_buffer;
    Cursor cursor;

    u16 top_line            = 0;    // number of the line that is currently printed as the top line
    LineBuffer lines;
};

} // namespace terminal

#endif /* SRC_SCREENPRINTER_H_ */
