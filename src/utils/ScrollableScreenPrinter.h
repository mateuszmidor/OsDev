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
    void format(char const *fmt, Args ... args) {
        const kstd::string& str = kstd::format(fmt, args...);
        for (const char& c : str)
            putc(c);
    }
    void putc(const char c);

protected:
    void putc_into_buffer(const char c);
    void scroll_down_if_needed();
    void redraw();
    void draw_scroll_bar();
    void put_line_and_clear_remaining_space_at(u8 y, const kstd::string& line);

    LineBuffer lines;
    u16 top_line            = 0;

    const char BG_CHAR      = 176;
    const char BG_SCROLLER  = 219;

private:
    bool is_edit_line_visible();
};

} // namespace utils

#endif /* SRC_SCREENPRINTER_H_ */
