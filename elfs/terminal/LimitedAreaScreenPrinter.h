/**
 *   @file: LimitedAreaScreenPrinter.h
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_LIMITEDAREASCREENPRINTER_H_
#define SRC_UTILS_LIMITEDAREASCREENPRINTER_H_

#include "types.h"
#include "ustd.h"
#include "Cursor.h"
#include "VgaCharacter.h"

namespace utils {

/**
 * This printer prints in given sub area of the VGA buffer
 */
class LimitedAreaScreenPrinter {
public:
    LimitedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom);
    virtual ~LimitedAreaScreenPrinter();
    void newline();
    void tab();
    void backspace();

    /**
     * @name    format
     * @example format("CPU: %", cpu_vendor_cstr);
     */
    template<typename ... Args>
    void format(char const *fmt, Args ... args) {
        const ustd::string& str = ustd::format(fmt, args...);
        for (const char& c : str)
            putc(c);
    }

protected:
    u16 vga_width      = 0;
    u16 vga_height     = 0;
    u16 left, top, right, bottom;   // printable area description
    Cursor cursor;
    u16 printable_area_width, printable_area_height;    // printable area dimension
    drivers::EgaColor foreground = drivers::EgaColor::White;
    drivers::EgaColor background = drivers::EgaColor::Black;

    void set_at(const Cursor& cursor, drivers::VgaCharacter c, bool autoflush = true);
    void set_at(u16 x, u16 y, drivers::VgaCharacter c, bool autoflush = true);
    void putc(const char c);
    void putc_into_vga(const char c);
    void flush_vga_buffer();

private:
    drivers::VgaCharacter* vga_buffer;

};

} /* namespace utils */

#endif /* SRC_UTILS_LIMITEDAREASCREENPRINTER_H_ */
