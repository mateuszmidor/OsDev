/**
 *   @file: ScreenPrinter.h
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#ifndef SRC_SCREENPRINTER_H_
#define SRC_SCREENPRINTER_H_

enum Color : unsigned char {
    Black      = 0,
    Blue       = 1,
    Green      = 2,
    Cyan       = 3,
    Red        = 4,
    Magenta    = 5,
    Brown      = 6,
    LightGray  = 7,
    DarkGray   = 8,
    LightBlue  = 9,
    LightGreen = 10,
    LightCyan  = 11,
    LightRed   = 12,
    Pink       = 13,
    Yellow     = 14,
    White      = 15,
};

class ScreenPrinter {
public:
    void set_fg_color(const Color value);
    void set_bg_color(const Color value);
    void clearscreen();

    void format(char const *fmt) {
        while (*fmt)
            putc(*fmt++);
    }

    /**
     * @name    format
     * @example format("CPU: %", cpu_vendor_cstr);
     */
    template<typename Head, typename ... Tail>
    void format(char const *fmt, Head head, Tail ... tail) {
        while (*fmt) {
            if (*fmt == '%') {
                format(head);
                format(++fmt, tail...);
                break;
            } else
                putc(*fmt++);
        }
    }

private:
    const unsigned int NUM_COLS = 80u;
    const unsigned int NUM_ROWS = 25u;
    unsigned short* vga = (unsigned short*)0xb8000;
    unsigned int cursor_pos = 0;
    Color foreground = Color::White;
    Color background = Color::Black;

    void newline();
    void putc(const char c);
};

#endif /* SRC_SCREENPRINTER_H_ */
