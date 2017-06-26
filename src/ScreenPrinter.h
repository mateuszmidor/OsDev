/**
 *   @file: ScreenPrinter.h
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#ifndef SRC_SCREENPRINTER_H_
#define SRC_SCREENPRINTER_H_

#include "types.h"
#include "VgaDriver.h"


/**
 * This is the structure describing single character in VGA buffer
 */
struct VgaCharacter {
    s8      character;
    u8      fg_color    : 4;
    u8      bg_color    : 4;
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
        u8 base = 10;
        char str[12];

            u8 i = 0;
            bool isNegative = false;

            /* Handle 0 explicitely, otherwise empty string is printed for 0 */
            if (num == 0)
            {
                format("0");
                return ;
            }

            // In standard itoa(), negative numbers are handled only with
            // base 10. Otherwise numbers are considered unsigned.
            if (num < 0 && base == 10)
            {
                isNegative = true;
                num = -num;
            }

            // Process individual digits
            while (num != 0)
            {
                int rem = num % base;
                str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
                num = num/base;
            }

            // If number is negative, append '-'
            if (isNegative)
                str[i++] = '-';

            str[i] = '\0'; // Append string terminator

            // Reverse the string
            u8 start = 0;
            u8 end = i -1;
            while (start < end)
            {

                auto tmp = *(str+start);
                *(str+start) = *(str+end);
                *(str+end) = tmp;
                start++;
                end--;
            }

        format(str);
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
