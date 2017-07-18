/**
 *   @file: ScreenPrinter.h
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#ifndef SRC_SCREENPRINTER_H_
#define SRC_SCREENPRINTER_H_

#include "types.h"
#include "kstd.h"
#include "VgaDriver.h"


/**
 * This is the structure describing single character in VGA buffer
 */
struct VgaCharacter {
    s8      character;
    u8      fg_color    : 4;
    u8      bg_color    : 4;
};

class BoundedAreaScreenPrinter {
public:
    BoundedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom, u16 vga_width, u16 vga_height);

    void format(s64 num) {
        format(kstd::to_str(num));
    }

    void format(const kstd::string& fmt) {
        format(fmt.c_str());
    }

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

protected:
    VgaCharacter* const vga = (VgaCharacter*)0xb8000;

    u16 left, top, right, bottom;   // printable area description
    u16 vga_width, vga_height;      // actual vga buffer dimension eg. 90x30
    u16 cursor_x, cursor_y;         // current cursor position in vga dimmension space
    drivers::EgaColor foreground = drivers::EgaColor::White;
    drivers::EgaColor background = drivers::EgaColor::Brown;

    void newline();
    void tab();
    void backspace();
    void putc(const char c);
};

class ScreenPrinter {
public:
    static ScreenPrinter& instance();

    VgaCharacter& at(u16 x, u16 y);
    void move_to(u16 x, u16 y);
    void swap_fg_bg_at(u16 x, u16 y);
    void set_fg_color(const drivers::EgaColor value);
    void set_bg_color(const drivers::EgaColor value);
    void clearscreen();


    void format(s64 num) {
        format(kstd::to_str(num));
    }

    void format(const kstd::string& fmt) {
        format(fmt.c_str());
    }

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
    const u16 NUM_COLS = 90u;
    const u16 NUM_ROWS = 30u;
    VgaCharacter* const vga = (VgaCharacter*)0xb8000;

    static ScreenPrinter _instance;
    u32 cursor_pos = 0;
    drivers::EgaColor foreground = drivers::EgaColor::White;
    drivers::EgaColor background = drivers::EgaColor::Black;


    void newline();
    void tab();
    void backspace();
    void putc(const char c);
};

#endif /* SRC_SCREENPRINTER_H_ */
