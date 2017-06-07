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

    void format(long long int num) {
        int base = 10;
        char str[12];

            int i = 0;
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
            int start = 0;
            int end = i -1;
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
