/*
 * VgaDemo.h
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#ifndef SRC__DEMOS_VGADEMO_H_
#define SRC__DEMOS_VGADEMO_H_

#include <memory>
#include "VgaDriver.h"

namespace demos {

class VgaDemo {
public:
    void run();

private:
    void on_mouse_down(u8 button);
    void on_mouse_move(s8 dx, s8 dy);
    s16 mouse_x, mouse_y;
    drivers::EgaColor pen_color = drivers::EgaColor::LightRed;
    std::shared_ptr<drivers::VgaDriver> vga;
};

} /* namespace demos */

#endif /* SRC__DEMOS_VGADEMO_H_ */
