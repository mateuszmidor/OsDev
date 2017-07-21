/*
 * VgaDemo.cpp
 *
 *  Created on: Jul 20, 2017
 *      Author: mateusz
 */

#include "VgaDemo.h"
#include "KernelLog.h"
#include "DriverManager.h"
#include "MouseDriver.h"

using namespace drivers;
namespace demos {

void VgaDemo::run() {
    auto& klog = KernelLog::instance();
    auto& driver_manager = DriverManager::instance();

    vga = driver_manager.get_driver<VgaDriver>();
    if (!vga) {
        klog.format("VgaDemo::run: no VgaDriver\n");
        return;
    }

    auto mouse = driver_manager.get_driver<MouseDriver>();
    if (!mouse) {
        klog.format("VgaDemo::run: no MouseDriver\n");
        return;
    }

    mouse_x = 320 / 2;
    mouse_y = 200 / 2;

    mouse->set_on_move([&](s8 dx, s8 dy) { on_mouse_move(dx, dy); });
    mouse->set_on_down([&](u8 button) { on_mouse_down(button); });
    vga->set_graphics_mode_320_200_256();

    for (u16 x = 0; x < vga->screen_width(); x++)
        for (u16 y = 0; y < vga->screen_height(); y++)
            vga->put_pixel(x, y, (x > 315 || x < 4 || y > 195 || y < 4) ? EgaColor::LightGreen : EgaColor::Black); // 4 pixels thick frame around the screen

    // keep task alive
    while (true)
        asm("hlt");
}

void VgaDemo::on_mouse_down(u8 button) {
    switch (button) {
        default:
        case MouseButton::LEFT:    pen_color = EgaColor::LightRed; break;
        case MouseButton::RIGHT:   pen_color = EgaColor::LightGreen; break;
        case MouseButton::MIDDLE:  pen_color = EgaColor::LightBlue; break;
    }
}

void VgaDemo::on_mouse_move(s8 dx, s8 dy) {
    const u16 MAX_X = vga->screen_width();
    const u16 MAX_Y = vga->screen_height();
    mouse_x += dx ;
    mouse_y += dy;
    if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > MAX_X) mouse_x = MAX_X; if (mouse_y > MAX_Y) mouse_y = MAX_Y;

    vga->put_pixel(mouse_x, mouse_y, pen_color);
}
} /* namespace demos */
