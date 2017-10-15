/**
 *   @file: Cursor.h
 *
 *   @date: Jul 26, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC_UTILS_CURSOR_H_
#define SRC_UTILS_CURSOR_H_

#include "types.h"
#include "kstd.h"
#include "VgaDriver.h"

namespace utils {

/**
 * @brief   Cursor that moves within limited area and automatically updates VGA cursor position
 */
class Cursor {
public:
    Cursor(u16 left, u16 top, u16 right, u16 bottom);
    u16 get_x() const;
    u16 get_y() const;
    void set_x(u16 x);
    void set_y(u16 y);
    void operator++(int);
    void operator--(int);
    void newline();
    void set_visible(bool visible);

private:
    void update_vga_cursor();
    drivers::VgaDriver* get_vga();
    drivers::VgaDriver* vga;
    u16 left, top, right, bottom;   // printable area description
    u16 cursor_x, cursor_y;         // current cursor position limited by [left, top, right, bottom]
};

} /* namespace utils */

#endif /* SRC_UTILS_CURSOR_H_ */
