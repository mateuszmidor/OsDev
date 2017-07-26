/**
 *   @file: ScreenPrinter.h
 *
 *   @date: Jun 5, 2017
 * @author: mateusz
 */

#ifndef SRC_SCREENPRINTER_H_
#define SRC_SCREENPRINTER_H_

#include <memory>
#include "types.h"
#include "kstd.h"
#include "VgaDriver.h"


namespace utils {

/**
 * This printer prints in given sub area of the VGA buffer
 */
class BoundedAreaScreenPrinter {
public:
    BoundedAreaScreenPrinter(u16 left, u16 top, u16 right, u16 bottom);
    virtual ~BoundedAreaScreenPrinter() {}
    void set_cursor_visible(bool visible);
    void set_cursor_pos(u8 x, u8 y);
    void newline();
    virtual void backspace();

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
    u16 left, top, right, bottom;   // printable area description
    u16 printable_area_width, printable_area_height;    // printable area dimension
    u16 cursor_x, cursor_y;         // current cursor position in vga dimension space
    drivers::EgaColor foreground = drivers::EgaColor::White;
    drivers::EgaColor background = drivers::EgaColor::Brown;

    drivers::VgaCharacter& at(u16 x, u16 y);
    virtual void putc(const char c);
    void putc_into_vga(const char c);
    void tab();
    std::shared_ptr<drivers::VgaDriver> get_vga();

private:
    std::shared_ptr<drivers::VgaDriver> vga;
};

class LineBuffer {
public:
    LineBuffer();
    u32 count() const;
    void push_back(const kstd::string& line);
    void putc(char c);
    void backspace();
    void newline();
    const kstd::string& back() const;
    const kstd::string& operator[](u32 index) const;

private:
    kstd::vector<kstd::string> lines;
};

class ScrollableScreenPrinter : public BoundedAreaScreenPrinter {
public:
    ScrollableScreenPrinter(u16 left, u16 top, u16 right, u16 bottom);
    virtual ~ScrollableScreenPrinter() {}

    void backspace() override;
    void scroll_up(u16 lines);
    void scroll_down(u16 lines);
    void scroll_to_begin();
    void scroll_to_end();
    void clear_screen();

protected:
    void putc(const char c) override;
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
