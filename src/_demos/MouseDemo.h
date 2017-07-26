/**
 *   @file: MouseDemo.h
 *
 *   @date: Jul 21, 2017
 * @author: Mateusz Midor
 */

#ifndef SRC__DEMOS_MOUSEDEMO_H_
#define SRC__DEMOS_MOUSEDEMO_H_

#include <memory>
#include "VgaDriver.h"

namespace demos {

class MouseDemo {
public:
    void run(u64 arg);

private:
    void on_mouse_down(u8 button);
    void on_mouse_move(s8 dx, s8 dy);
    void swap_fg_bg_at(u16 x, u16 y);

    s16 mouse_x = 360;
    s16 mouse_y = 200;
    drivers::EgaColor pen_color = drivers::EgaColor::LightRed;
    std::shared_ptr<drivers::VgaDriver> vga;
};

} /* namespace demos */

#endif /* SRC__DEMOS_MOUSEDEMO_H_ */
