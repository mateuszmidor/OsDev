/**
 *   @file: ScrollableScreenPrinter.h
 *
 *   @date: Jun 5, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_SCREENPRINTER_H_
#define SRC_SCREENPRINTER_H_

#include "LimitedAreaScreenPrinter.h"
#include "LineBuffer.h"

namespace utils {


class ScrollableScreenPrinter : public LimitedAreaScreenPrinter {
public:
    ScrollableScreenPrinter(u16 left, u16 top, u16 right, u16 bottom);
    virtual ~ScrollableScreenPrinter() {}

    void newline();
    void backspace();
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
        const ustd::string& str = ustd::format(fmt, args...);
        for (const char& c : str)
            putc(c);
            redraw();
    }
    void putc(const char c);

protected:
    void putc_into_buffer(const char c);
    void redraw();
    void draw_scroll_bar();
    void put_line_and_clear_remaining_space_at(u8 y, const ustd::string& line);
    bool is_edit_line_visible();

    LineBuffer lines;
    u16 top_line            = 0;

    const char BG_CHAR      = 176;
    const char BG_SCROLLER  = 219;
};

} // namespace utils

#endif /* SRC_SCREENPRINTER_H_ */
