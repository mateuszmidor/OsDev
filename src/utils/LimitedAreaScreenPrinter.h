/**
 *   @file: LimitedAreaScreenPrinter.h
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_LIMITEDAREASCREENPRINTER_H_
#define SRC_UTILS_LIMITEDAREASCREENPRINTER_H_

#include <memory>
#include "types.h"
#include "kstd.h"
#include "VgaDriver.h"
#include "Cursor.h"

namespace utils {

/**
 * This printer prints in given sub area of the VGA buffer
 */
class LimitedAreaScreenPrinter {
public:
    LimitedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom);
    virtual ~LimitedAreaScreenPrinter() {}
    void newline();
    void tab();
    void backspace();

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

protected:
    Cursor cursor;
    u16 left, top, right, bottom;   // printable area description
    u16 printable_area_width, printable_area_height;    // printable area dimension
    drivers::EgaColor foreground = drivers::EgaColor::White;
    drivers::EgaColor background = drivers::EgaColor::Brown;

    drivers::VgaCharacter& at(const Cursor& cursor);
    drivers::VgaCharacter& at(u16 x, u16 y);
    void putc(const char c);
    void putc_into_vga(const char c);
    std::shared_ptr<drivers::VgaDriver> get_vga();

private:
    std::shared_ptr<drivers::VgaDriver> vga;
};

} /* namespace utils */

#endif /* SRC_UTILS_LIMITEDAREASCREENPRINTER_H_ */
